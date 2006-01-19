// vim:ts=4:sw=4:noet
#include "InterHub.h"
#include "Hub.h"
#include "Encoder.h"
#include "TigerHash.h"
#include "UserInfo.h"
#include "Plugin.h"

#include "qhub.h"
#include "error.h"
#include "Logs.h"
#include "Util.h"

#include <netinet/in.h>
#include <memory>

using namespace std;
using namespace qhub;


class DNSLookup: public DNSAdapter
{
	public:
        DNSLookup(InterHub* ih, string s) : DNSAdapter(s), interHub(ih) {}
        virtual void complete(const string& r)
        {
			interHub->onLookup(r);
		}
	
	protected:
		InterHub *interHub;
};


InterHub::InterHub(Hub* h, const string& hn, short p) throw()
		: ADCSocket(h), hostname(hn), port(p), outgoing(true)
{
	new DNSLookup(this, hn);
}

InterHub::InterHub(Hub* h, int fd, Domain d) throw()
		: ADCSocket(fd, d, h), outgoing(false) {}

void InterHub::onLookup(const string& ip)
{
	Logs::stat << "Connecting to hub at ip " << ip << " and port " << getPort() << endl;
	assert(getState() == PROTOCOL);

	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(getPort());
	//dest_addr.sin_addr = reply->rrs.inaddr[0];
	dest_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	memset(&(dest_addr.sin_zero), '\0', 8);

	::connect(getFd(), reinterpret_cast<sockaddr*>(&dest_addr), sizeof(sockaddr));
	enableMe(ev_read);
	doSupports();

}

void InterHub::onConnected() throw()
{
	timer = Timer::makeTimer(15);
	timer->addListener(this);
	Plugin::InterConnected action;
	Plugin::fire(action, this);
}

void InterHub::onDisconnected(const string& clue) throw()
{
	if(getState() != NORMAL)
		return;
	getHub()->deactivate(this);
	for(Users::iterator i = users.begin(); i != users.end(); ++i) {
		getHub()->broadcast("IQUI " + getHub()->getCID32() + ' ' + i->first + '\n');
		delete i->second;
	}
	users.clear();
	Plugin::InterDisconnected action;
	Plugin::fire(action, this);
}

bool InterHub::hasClient(const string& cid) const
{
	return users.find(cid) != users.end();
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
	send("LSTA " + getHub()->getCID32() + " 200 " + ADC::ESC(msg) + '\n');
}

void InterHub::onLine(StringList& sl, const string& fullmsg) throw(command_error)
{
	if(timer) {
		timer->removeListener(this);
		timer = NULL;
	}

	if(sl[0].size() != 4) {
		disconnect("bad FOURCC");
		return;
	}
	
	uint32_t command = stringToFourCC(sl[0]);
	string full = fullmsg;

	{
		Plugin::InterLine action;
		Plugin::fire(action, this, command, sl);
		if(action.isSet(Plugin::DISCONNECTED) || action.isSet(Plugin::STOPPED)) {
			return;
		} else if(action.isSet(Plugin::MODIFIED)) {
			command = stringToFourCC(sl[0]);
			ADC::toString(sl, full);
		}
	}

	switch(state) {
	case PROTOCOL:
		if(command != (SUP | 'L')) {
			disconnect("bad command type in state PROTOCOL");
			return;
		}
		if (sl.size() < 4 || find(sl.begin()+2, sl.end(), "+BASE") == sl.end() ||
				find(sl.begin()+2, sl.end(), "+IHUB") == sl.end()) {
			disconnect("invalid supports");
			return;
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
		cid = sl[1];
		if(!ADC::checkCID(cid)) {
			disconnect("InterHub::onLine invalid CID: " + sl[1]);
			return;
		}
		//TODO: verify INF stuff
		doAskPassword();
		state = VERIFY;
		break;
	case VERIFY:
		if(command != (PAS | 'L') && command != (GPA | 'L')) {
			doError("bad command type");
			disconnect("InterHub::onLine bad command type in state VERIFY");
			return;
		}
		if((command & 0xFFFFFF00) == GPA) {
			doPassword(sl);
		} else if((command & 0xFFFFFF00) == PAS) {
			handlePassword(sl);
			state = NORMAL;
			getHub()->getUserList(this);
			getHub()->activate(this);
		} else
			assert(0);
		break;
	case NORMAL:
		//pass the message on
		handle(sl, full, command);
		break;
	default:
		assert(0);
	}
}

void InterHub::doSupports() throw()
{
	send("LSUP " + getHub()->getCID32() + " +BASE +IHUB\n");
}

void InterHub::doInf() throw()
{
	send("SINF " + getHub()->getCID32() + " NI" + ADC::ESC(getHub()->getHubName()) +
			" HU1 DE" + ADC::ESC(getHub()->getDescription()) + " VE"
			PACKAGE_NAME "/" PACKAGE_VERSION "\n");
}

void InterHub::doAskPassword() throw()
{
	assert(state == IDENTIFY && salt.empty());
	salt = Util::genRand(24);
	send("LGPA " + getHub()->getCID32() + ' ' + Encoder::toBase32(&salt.front(), salt.size()) + '\n');
}

void InterHub::doPassword(const StringList& sl) throw()
{
	assert(state == VERIFY);
	size_t len = sl[2].size() * 5 / 8;
	uint8_t* s = new uint8_t[len];
	Encoder::fromBase32(sl[2].data(), s, len);
	TigerHash h;
	h.update(getHub()->getCID32().data(), getHub()->getCID32().size());
	h.update(getHub()->getInterPass().data(), getHub()->getInterPass().size());
	h.update(s, len);
	delete[] s;
	send("LPAS " + getHub()->getCID32() + ' ' +
			Encoder::toBase32(h.finalize(), TigerHash::HASH_SIZE) + '\n');
}

void InterHub::handle(const StringList& sl, const string& full, uint32_t command) throw(command_error)
{
	switch(full[0]) {
	case 'B':
		if(command == (INF | 'B')) {
			if(users.find(sl[1]) == users.end()) {
				users.insert(make_pair(sl[1], new UserInfo(this, sl)));
			} else {
				users.find(sl[1])->second->update(UserInfo(this, sl));
			}
		}
		getHub()->broadcast(full, this);
		break;
	case 'F':
		if(sl[2].size() != 5 || sl[2][0] != '+' || sl[2][0] != '-')
			throw command_error("Feature parameter corrupt:\n" + full);
		getHub()->broadcastFeature(full, sl[2].substr(1), sl[2][0] == '+', this);
		break;
	case 'A':
		getHub()->broadcastActive(full, this);
		break;
	case 'P':
	case 'T': // FIXME
		getHub()->broadcastPassive(full, this);
		break;
	case 'S':
		getHub()->broadcastInter(full, this);
	case 'L':
		if(sl[1] != cid) {
			doError("CID mismatch");
			disconnect("CID mismatch");
		}
		break;
	case 'I':
		{
			string tmp(full);
			tmp.replace(5, 13, getHub()->getCID32());
			getHub()->broadcast(tmp, this);
			if(command == ('I' | QUI)) {
				Users::iterator i = users.find(sl[2]);
				if(i == users.end())
					break;
				delete i->second;
				users.erase(i);
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
	h.update(getHub()->getInterPass().data(), getHub()->getInterPass().size());
	h.update(&salt.front(), salt.size());
	h.finalize();
	string result32 = Encoder::toBase32(h.getResult(), TigerHash::HASH_SIZE);
	if(result32 != sl[2]) {
		send("LSTA " + getHub()->getCID32() + " 223 " + ADC::ESC("Bad password") + '\n');
		disconnect("bad password: expected " + result32);
		return;
	}
	salt.clear();
}
