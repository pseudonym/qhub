// vim:ts=4:sw=4:noet
#include "ADC.h"
#include "Hub.h"

using namespace std;
using namespace qhub;

ADC::ADC(int fd, Hub* parent)
: ADCSocket(fd), hub(parent), state(START), added(false)
{
}

ADC::~ADC()
{
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
	switch(sl[0][0]) {
	case 'H':
		if(sl[0] == "HSUP") {
			if(state == START) {
				send(string("ISUP ") + hub->getCID32() + " +BASE\n"
						"IINF " + hub->getCID32() + " NI" + esc(hub->getHubName()) +
						" HU1 HI1 DEmajs VEqhub0.02\n");
				state = GOT_SUP;
			} else {
			}
		} else {
			fprintf(stderr, "Got H: %s\n", sl[0].c_str());
		}
		break;
	case 'B':
		if(sl[0] == "BINF") {
			if(sl.size() >= 2) {
				guid = sl[1];
				for(StringList::const_iterator sli = sl.begin() + 2; sli != sl.end(); ++sli) {
					if(sli->length() >= 2) {
						// FIXME no excessive substr
						INF[sli->substr(0, 2)] = sli->substr(2);
						if(sli->substr(0, 2) == "I4" && sli->substr(2) == "0.0.0.0") {
							INF[sli->substr(0, 2)] = getPeerName();
						}
					}
				}
				hub->getUsersList(this);
				//add us later, dont want us two times
				if(!hub->addClient(this, guid)) {
					PROTO_DISCONNECT("proto error: User exists already");
					return;
				}
				//only set this when we are sure that we are added, ie. here!
				added = true;

				//notify him that userlist is over
				sendFullInf();
				state = LOGGED_IN;
				hub->motd(this);
				//XXX: when DC++ isnt buggy
				//namedParms.erase(namedParms.find("LO"));
			} else {
				state = PROTOCOL_ERROR;
#ifdef PROTO_DEBUG
				sendHubMessage("proto error: BINF: no GUID supplied");
#endif
				disconnect();
			}
		} else {
			fprintf(stderr, "Broadcasting B: %s\n", sl[0].c_str());
			hub->broadcastSelf(this, full);
		}
		break;
	default:
		fprintf(stderr, "Broadcasting something: %s\n", sl[0].c_str());
		hub->broadcastSelf(this, full);
	}
}
#undef PROTO_DISCONNECT

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

string ADC::getFullInf()
{
	string parms = "";
	for(INFIterator i = INF.begin(); i!=INF.end(); i++){
		parms += " " + i->first + i->second;
	}
	return "BINF " + guid + parms + "\n";
}

void ADC::sendFullInf()
{
	hub->broadcastSelf(this, getFullInf());
}

void ADC::sendHubMessage(string const& msg)
{
	send(string("BMSG ") + hub->getCID32() + ' ' + esc(msg) + '\n');
}

