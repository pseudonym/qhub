// vim:ts=4:sw=4:noet
#include "ADC.h"
#include "Hub.h"

using namespace std;
using namespace qhub;

ADC::ADC(int fd, Hub* parent)
: ADCSocket(fd), attributes(this), hub(parent), state(START), added(false)
{
	onConnected();
}

ADC::~ADC()
{
}

void ADC::enable()
{
	hub->addClient(guid, this);
	added = true;
}

void ADC::cancel()
{
	hub->removeClient(guid);
	added = false;
}

void ADC::sendHubMessage(string const& msg)
{
	send("BMSG " + hub->getCID32() + ' ' + esc(msg) + '\n');
}


#define PROTOCOL_ERROR(errmsg) \
	do { \
		doError(errmsg); \
		if(added) { \
			cancel(); \
			hub->broadcastSelf("IQUI " + guid + " DI " + esc(errmsg) + '\n'); \
		} \
		disconnect(); \
	} while(0)


/************************/
/* Calls from ADCSocket */
/************************/

void ADC::doWarning(string const& msg) {
	send("ISTA " + guid + " 10 " + esc(msg) + '\n');
}

void ADC::doError(string const& msg) {
	send("ISTA " + guid + " 20 " + esc(msg) + '\n');
}

void ADC::onLine(StringList const& sl, string const& full)
{		
	assert(!sl.empty());
	fprintf(stderr, "<< %s", full.c_str());
	// Do basic input checking, state and guid
	if(sl[0].length() == 4) {
		switch(sl[0][0]) {
		case 'A':
			if(state == NORMAL && sl.size() >= 2 && sl[1] == guid) {
				handleA(sl, full);
			} else {
				PROTOCOL_ERROR("State, CID or parameter mismatch");
			}
			break;
		case 'B':
			if(sl.size() >= 2 && ((state == NORMAL && sl[1] == guid) || state == IDENTIFY)) {
				handleB(sl, full); // check state
			} else {
				PROTOCOL_ERROR("State, CID or parameter mismatch");
			}
			break;
		case 'C': 
			PROTOCOL_ERROR("C type messages not supported");
			return;
		case 'D':
			if(state == NORMAL && sl.size() >= 3 && sl[2] == guid) {
				handleD(sl, full);
			} else {
				PROTOCOL_ERROR("State, CID or parameter mismatch");
			}
			break;
		case 'I':
			PROTOCOL_ERROR("I type messages not supported");
			return;
		case 'H': // XXX do H messages have a CID or not? may affect BINF state checking
			handleH(sl, full); // check state
			break;
		case 'P':
			if(state == NORMAL && sl.size() >= 2 && sl[1] == guid) {
				handleD(sl, full);
			} else {
				PROTOCOL_ERROR("State, CID or parameter mismatch");
			}
			break;
		case 'U':
			PROTOCOL_ERROR("U type messages not supported");
			break;
		default:
			PROTOCOL_ERROR("Message type not supported");
		}
	} else {
		PROTOCOL_ERROR("Illegal message");
	}
}

void ADC::onConnected()
{
	// dunno.. do we need this? ;)
	// plugins might.. to check IP for instance
}

void ADC::onDisconnected(string const& clue)
{
	if(added) {
		// this is here so ADCSocket can safely destroy us.
		// if we don't want a second message and our victim to get the message as well
		// remove us when doing e.g. the Kick, so that added is false here.
		fprintf(stderr, "onDisconnected %d %p GUID: %s\n", fd, this, guid.c_str());
		cancel();
		if(clue.empty())
			hub->broadcast(this, string("IQUI " + guid + " ND\n"));
		else
			hub->broadcast(this, string("IQUI " + guid + " DI " + hub->getCID32() + ' ' + esc(clue) + '\n'));
	}
}



/*****************/
/* Data handlers */
/*****************/

void ADC::handleA(StringList const& sl, string const& full)
{
	// todo: check parameters
	if(sl[0] == "AMSG") {
		hub->broadcastSelf(full);
	} else {
		PROTOCOL_ERROR(sl[0] + " message type or parameter count unsupported");
	}
}
	
void ADC::handleB(StringList const& sl, string const& full)
{
	if(state == IDENTIFY) {
		if(sl[0] == "BINF") {
			handleBINF(sl, full);
		} else {
			PROTOCOL_ERROR("State mismatch");
		}
	} else if(state == NORMAL) {
		if(sl[0] == "BINF") {
			handleBINF(sl, full);
		} else if(sl[0] == "BMSG" && (sl.size() == 3 || sl.size() == 4 /* flags */)) {
			handleBMSG(sl, full);
		} else if(sl[0] == "BSCH") {
			hub->broadcast(this, full);
		} else {
			PROTOCOL_ERROR(sl[0] + " message type or parameter count unsupported");
		}
	} else {
		PROTOCOL_ERROR("State mismatch");
	}
}

void ADC::handleBINF(StringList const& sl, string const& full)
{
	if(!attributes.setInf(sl)) {
		PROTOCOL_ERROR("setInf didn't like you very much");
		return;
	}
	if(state == IDENTIFY) {
		guid = sl[1];
		if(hub->hasClient(guid)) {
			PROTOCOL_ERROR("CID busy, change CID or wait");
			return;
		}
		//send infs
		hub->getUsersList(this);

		enable();
		//notify him that userlist is over and notify others of his presence
		hub->broadcastSelf(attributes.getChangedInf());
		state = NORMAL;
		hub->motd(this);
	} else {
		hub->broadcastSelf(attributes.getChangedInf());
	}
}
	
void ADC::handleBMSG(StringList const& sl, string const& full)
{
	if(sl[2].length() >= 9 && sl[2].substr(0, 7) == "setInf ") {
		attributes.setInf(sl[2].substr(7, 2), sl[2].substr(9));
		hub->broadcastSelf(attributes.getChangedInf());
	}
	hub->broadcastSelf(full);
	// FIXME add check for PM<guid> flag
}

void ADC::handleD(StringList const& sl, string const& full)
{
	// todo: check parameter count
	if(
			sl[0] == "DSTA" ||
			sl[0] == "DMSG" ||
			sl[0] == "DSCH" ||
			sl[0] == "DRES" ||
			sl[0] == "DCTM" ||
			sl[0] == "DRCM"
	) {
		hub->broadcastSelf(full);
	} else {
		PROTOCOL_ERROR(sl[0] + " message type or parameter count unsupported");
	}
}
	
void ADC::handleH(StringList const& sl, string const& full)
{
	if(sl[0] == "HSUP") {
		handleHSUP(sl, full); // valid in all states
	} else if(state == VERIFY && sl[0] == "HPAS") {
		// FIXME
	} else if(state == NORMAL) {
		if(sl[0] == "HDSC") {
			handleHDSC(sl, full);
		} else if(sl[0] == "HGET" || sl[0] == "HSND") {
			doWarning("HGET/HSND unsupported");
		} else {
			PROTOCOL_ERROR(sl[0] + " message type or parameter count unsupported");
		}
	} else {
		PROTOCOL_ERROR("State mismatch");
	}
}

void ADC::handleHDSC(StringList const& sl, string const& full)
{
	// H didn't have cid...
	if(!attributes.isOp()) {
		doWarning("Access denied");
		return;
	}
	if(sl.size() >= 5) {
		bool hide = sl[2] == "ND";
		string const& victim_guid = sl[3];
		if(!hide && sl[1] != sl[2]) {
			PROTOCOL_ERROR("HDSC corrupt"); // "ND" or sl[1] should be at sl[2]
			return;
		}
		ADC* victim = hub->getClient(victim_guid);
		if(!victim)
			return;
		bool success = false;
		string msg = string("IQUI ") + victim_guid;
		if(sl.size() == 5) {
			if(sl[1] == "DI") {
				msg += " DI " + guid + ' ' + esc(sl[4]) + '\n';
				victim->send(msg);
				success = true;
			} else if(sl[1] == "KK") {
				msg += " KK " + guid + ' ' + esc(sl[4]) + '\n';
				victim->send(msg);
				// todo: set bantime
				success = true;
			}
		} else if(sl.size() == 6) {
			if(sl[1] == "BN") {
				msg += " BN " + guid + ' ' + esc(sl[4]) + ' ' + esc(sl[5]) + '\n';
				victim->send(msg);
				// todo: set bantime
				success = true;
			} else if(sl[1] == "RD") {
				msg += " RD " + guid + ' ' + esc(sl[4]) + ' ' + esc(sl[5]) + '\n';
				victim->send(msg);
				success = true;
			}
		}
		if(success) {
			// remove victim
			victim->cancel();
			victim->disconnect();
			// notify everyone else
			if(!hide) {
				hub->broadcastSelf(msg);
			} else {
				if(this != victim)
					send(msg); // notify self
				hub->broadcast(this, "IQUI " + victim_guid + " ND\n");
			}
		} else {
			doWarning("HDSC corrupt");
		}
	}
}
	
void ADC::handleHSUP(StringList const& sl, string const& full)
{
	send("ISUP " + hub->getCID32() + " +BASE\n"
			"IINF " + hub->getCID32() + " NI" + esc(hub->getHubName()) +
			" HU1 HI1 DEmajs VEqhub0.02\n");
	state = IDENTIFY;
}
	
void ADC::handleP(StringList const& sl, string const& full)
{
	// todo: check parameter count
	if(
			sl[0] == "PMSG" ||
			sl[0] == "PSCH"
	) {
		// FIXME
		hub->broadcastSelf(full);
	} else {
		PROTOCOL_ERROR(sl[0] + " message type or parameter count unsupported");
	}	
}
