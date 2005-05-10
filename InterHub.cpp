#include "InterHub.h"
#include "Hub.h"
#include "Encoder.h"
#include "TigerHash.h"
#include "UserInfo.h"

#include "qhub.h"
#include "error.h"
#include "string8.h"
#include "Util.h"

#include <netinet/in.h>
#include <memory>

using namespace std;
using namespace qhub;

StringMap InterHub::passes;

InterHub::InterHub(Hub* h, const string& hn, short p) throw()
		: ADCSocket(h), hostname(hn), port(p), outgoing(true)
{
	lookup(hostname.c_str());
}

InterHub::InterHub(Hub* h, int fd, Domain d) throw()
		: ADCSocket(fd, d, h), outgoing(false) {}

const string& InterHub::getPass(const string& cid) throw()
{
	StringMap::iterator i = passes.find(cid);
	if(i != passes.end())
		return i->second;
	return Util::emptyString;
}

void InterHub::onLookup(adns_answer* reply)
{
	assert(getState() == PROTOCOL);
	log(qstat, string("InterHub::onLookup: ") + reply->owner);

	if (reply->status != adns_s_ok) {
		log(qerr, string("InterHub::onLookup error: ") + adns_strerror(reply->status));
	} else {
		assert(reply->type == adns_r_a);
		if(reply->nrrs > 0){
			struct sockaddr_in dest_addr;
			dest_addr.sin_family = AF_INET;
			dest_addr.sin_port = htons(getPort());
			dest_addr.sin_addr.s_addr = inet_addr(inet_ntoa(reply->rrs.inaddr[0]));
			memset(&(dest_addr.sin_zero), '\0', 8);

			::connect(getFd(), (struct sockaddr*)&dest_addr, sizeof(struct sockaddr));
			enable_fd(getFd(), OOP_READ, this);
			doSupports();
		}
	}
}

void InterHub::onConnected() throw()
{
	alarm(15);
}

void InterHub::onDisconnected(const string& clue) throw()
{
	if(getState() == NORMAL)
		getHub()->deactivate(this);
	for(Users::iterator i = users.begin(); i != users.end(); ++i) {
		getHub()->broadcast(
				"IQUI " + getHub()->getCID32() + ' ' + i->first + '\n',
				NULL, true);
		delete i->second;
	}
}

bool InterHub::hasClient(const string& cid) const
{
	if(users.find(cid) == users.end())
		return false;
	return true;
}

void InterHub::appendUserList(string& tmp) throw()
{
	for(Users::iterator i = users.begin(); i != users.end(); ++i) {
		tmp += i->second->toADC(i->first);
	}
}

void InterHub::doError(const string& msg) throw()
{
	if(state == PROTOCOL)
		// silent disconnect... we don't want probes
		return;
	send("SSTA " + getHub()->getCID32() + " 200 " + ADC::ESC(msg) + '\n');
}

void InterHub::disconnect(const string& msg)
{
	ADCSocket::disconnect(msg);
}

void InterHub::onLine(StringList& sl, const string& full) throw()
{
	alarm(0);

	if(sl[0].size() != 4) {
		disconnect("bad FOURCC");
		return;
	}
	uint32_t command = stringToFourCC(sl[0]);

	switch(state) {
	case PROTOCOL:
		if(command != (SUP | 'S')) {
			disconnect("InterHub::onLine bad command type in state PROTOCOL");
			return;
		}
		if (sl.size() < 4 || find(sl.begin()+2, sl.end(), "+BASE") == sl.end() ||
				find(sl.begin()+2, sl.end(), "+IHUB") == sl.end()) {
			disconnect("InterHub::onLine invalid supports");
			return;
		}
		if(!ADC::checkCID(sl[1])) {
			disconnect("InterHub::onLine invalid CID: " + sl[1]);
			return;
		}
		cid = sl[1];
		if(getPass(cid).empty()) {
			//we don't know this hub, so disconnect
			disconnect("unauthorized CID");
		}
		if(!outgoing)
			doSupports();
		state = IDENTIFY;
		doInf();
		break;
	case IDENTIFY:
		if(command != (INF | 'S')) {
			doError("bad command type");
			disconnect("InterHub::onLine bad command type in state IDENTIFY");
			return;
		}
		//TODO: verify INF stuff
		doAskPassword();
		state = VERIFY;
		break;
	case VERIFY:
		if(command != (PAS | 'S') && command != (GPA | 'S')) {
			doError("bad command type");
			disconnect("InterHub::onLine bad command type in state VERIFY");
			return;
		}
		if((command & 0xFFFFFF00) == GPA) {
			doPassword(sl);
		} else if((command & 0xFFFFFF00) == PAS) {
			handlePassword(sl);
			state = NORMAL;
			getHub()->getUserList(this, true);
			getHub()->activate(this);
		} else
			assert(0 && "bad command type slipped through");
		break;
	case NORMAL:
		//pass the message on
		handle(sl, full, command);
		break;
	default:
		assert(0 && "InterHub::onLine invalid state");
	}
}

void InterHub::doSupports() throw()
{
	send("SSUP " + getHub()->getCID32() + " +BASE +IHUB\n");
}

void InterHub::doInf() throw()
{
	send("SINF " + getHub()->getCID32() + " NI" + ADC::ESC(getHub()->getHubName()) +
			" HU1 DE" + ADC::ESC(getHub()->getDescription()) + " VE" PACKAGE "/" VERSION "\n");
}

void InterHub::doAskPassword() throw()
{
	assert(state == IDENTIFY && salt.empty());
	salt = Util::genRand192();
	send("SGPA " + getHub()->getCID32() + ' ' + Encoder::toBase32(salt.data(), salt.size()) + '\n');
}

void InterHub::doPassword(const StringList& sl) throw()
{
	assert(state == VERIFY);
	size_t len = sl[2].size() * 5 / 8; // shouldn't need all of it, but not important
	uint8_t* s = new uint8_t[len];
	Encoder::fromBase32(sl[2].data(), s, len);
	TigerHash h;
	h.update(getHub()->getCID32().data(), getHub()->getCID32().size());
	h.update(getPass(getCID32()).data(), getPass(getCID32()).size());
	h.update(s, len);
	delete[] s;
	send("SPAS " + getHub()->getCID32() + ' ' +
			Encoder::toBase32(h.finalize(), TigerHash::HASH_SIZE) + '\n');
}

void InterHub::handle(const StringList& sl, const string& full, uint32_t command) throw()
{
	switch(full[0]) {
	case 'B':
		getHub()->broadcast(full, NULL, true);
		if(command == (INF | 'B')) {
			if(users.find(sl[1]) == users.end()) {
				users.insert(make_pair(sl[1], new UserInfo(this, sl)));
			} else {
				users.find(sl[1])->second->update(UserInfo(this, sl));
			}
		}
		break;
	case 'A':
		getHub()->broadcastActive(full, true);
		break;
	case 'P':
		getHub()->broadcastPassive(full, true);
		break;
	case 'S':
		if(sl[1] != cid) {
			doError("CID mismatch");
			disconnect("CID mismatch");
		}
		if(sl[0] == "SQUI") {
			Users::iterator i;
			if((i = users.find(sl[2])) != users.end()) {
				delete i->second;
				users.erase(i);
				string tmp(full);
				tmp.replace(5,13,getHub()->getCID32());
				tmp[0] = 'I';
				getHub()->broadcast(tmp, NULL, true);
			}
		}
		break;
	case 'D':
		if(!getHub()->hasClient(sl[2]))
			return;
		getHub()->direct(sl[2], full);
		break;
	default:
		break;
	}
}

void InterHub::handlePassword(const StringList& sl) throw()
{
	TigerHash h;
	h.update(getCID32().data(), getCID32().size());
	h.update(getPass(getHub()->getCID32()).data(), getPass(getHub()->getCID32()).size());
	h.update(salt.data(), salt.size());
	h.finalize();
	string result32 = Encoder::toBase32(h.getResult(), TigerHash::HASH_SIZE);
	if(result32 != sl[2]) {
		send("SSTA " + getHub()->getCID32() + " 223 " + ADC::ESC("Bad password") + '\n');
		disconnect("bad password: expected " + result32);
		return;
	}
	salt.clear();
}
