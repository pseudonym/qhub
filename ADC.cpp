// vim:ts=4:sw=4:noet
#include "ADC.h"
#include "Hub.h"

using namespace std;
using namespace qhub;

ADC::ADC(int fd, Hub* parent)
: attributes(this), ADCSocket(fd), hub(parent), state(START), added(false)
{
}

ADC::~ADC()
{
}

void ADC::sendHubMessage(string const& msg)
{
	send(string("BMSG ") + hub->getCID32() + ' ' + esc(msg) + '\n');
}


#define PROTO_DISCONNECT(errmsg) \
	do { \
		state = PROTOCOL_ERROR; \
		sendHubMessage("proto error: " errmsg); \
		disconnect(); \
	} while(0)

void ADC::onLine(StringList const& sl, string const& full)
{		
	assert(!sl.empty());
	fprintf(stderr, ">%s<\n", full.c_str());
	// Do basic input checking, state and guid
	if(sl[0].length() == 4) {
		switch(sl[0][0]) {
		case 'A':
			if(state == LOGGED_IN && sl.size() >= 2 && sl[1] == guid) {
				handleA(sl, full);
			} else {
				PROTO_DISCONNECT("Wrong GUID or Not logged in");
			}
			break;
		case 'B':
			if(sl.size() >= 2 && ((state == LOGGED_IN && sl[1] == guid) || state == GOT_SUP)) {
				handleB(sl, full);
			} else {
				PROTO_DISCONNECT("Wrong GUID or Not logged in");
			}
			break;
		case 'C': 
			PROTO_DISCONNECT("C messages are supposed to go to clients");
			return;
		case 'D':
			if(state == LOGGED_IN && sl.size() >= 3 && sl[2] == guid) {
				handleD(sl, full);
			} else {
				PROTO_DISCONNECT("Wrong GUID or Not logged in");
			}
			break;
		case 'I':
			PROTO_DISCONNECT("I messages originate from me, not from you");
			return;
		case 'H': // XXX do H messages have a CID or not? may affect BINF state checking
			handleH(sl, full);
			break;
		case 'P':
			if(state == LOGGED_IN && sl.size() >= 2 && sl[1] == guid) {
				handleD(sl, full);
			} else {
				PROTO_DISCONNECT("Wrong GUID or Not logged in");
			}
			break;
		case 'U':
			PROTO_DISCONNECT("U messages are supposed to be sent over UDP");
			break;
		default:
			PROTO_DISCONNECT("Message type unknown");
		}
	} else {
		PROTO_DISCONNECT("Illegal input");
	}
}

void ADC::onLineError(string const& msg) {
	sendHubMessage(msg);
	disconnect();
}

void ADC::onDisconnect() {
	if(added) {
		fprintf(stderr, "Disconnecting %d %p GUID: %s\n", fd, this, guid.c_str());
		hub->removeClient(guid);
		hub->broadcast(this, string("IQUI " + guid + " ND\n"));
		added = false;
	} 
}

void ADC::handleA(StringList const& sl, string const& full)
{
	hub->broadcastSelf(full);
}
	
void ADC::handleB(StringList const& sl, string const& full)
{
	if(sl[0] == "BINF") {
		handleBINF(sl, full);
	} else if(sl[0] == "BMSG") {
		if(sl.size() == 3) {
			if(sl[2].length() >= 9 && sl[2].substr(0, 7) == "setInf ") {
				attributes.setInf(sl[2].substr(7, 2), sl[2].substr(9));
				hub->broadcastSelf(string("BINF ") + guid + " " + sl[2].substr(7) +   + "\n");
			}
			// default
			hub->broadcastSelf(full);
		} else {
			PROTO_DISCONNECT("BMSG takes 2 parms only");
		}
	} else if(sl[0] == "BSCH") {
		// searches do not go to self
		hub->broadcast(this, full);	
	} else {
		hub->broadcastSelf(full);
	}
}

void ADC::handleBINF(StringList const& sl, string const& full)
{
	if(!attributes.setInf(sl)) {
		PROTO_DISCONNECT("setInf didn't like you very much");
		return;
	}
	if(state == GOT_SUP) {
		guid = sl[1];
		if(hub->hasClient(guid)) {
			PROTO_DISCONNECT("proto error: User exists already");
			return;
		}
		//send infs
		hub->getUsersList(this);
		//add us later, dont want us two times
		hub->addClient(guid, this);
		//only set this when we are sure that we are added, ie. here!
		added = true;
		//notify him that userlist is over and notify others of his presence
		hub->broadcastSelf(attributes.getChangedInf());
		state = LOGGED_IN;
		hub->motd(this);
	} else {
		hub->broadcastSelf(attributes.getChangedInf());
	}
}
	
void ADC::handleD(StringList const& sl, string const& full)
{
	hub->broadcastSelf(full);
}
	
void ADC::handleH(StringList const& sl, string const& full)
{
	if(sl[0] == "HSUP") {
		handleHSUP(sl, full);
	} else {
		// do not broadcast H
		sendHubMessage(full);
	}
}

void ADC::handleHSUP(StringList const& sl, string const& full)
{
	if(state == START) {
		send(string("ISUP ") + hub->getCID32() + " +BASE\n"
				"IINF " + hub->getCID32() + " NI" + esc(hub->getHubName()) +
				" HU1 HI1 DEmajs VEqhub0.02\n");
		state = GOT_SUP;
	} else {
		PROTO_DISCONNECT("HSUP unexpected");
	}
}
	
void ADC::handleP(StringList const& sl, string const& full)
{
	hub->broadcastSelf(full);
}
