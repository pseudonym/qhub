// vim:ts=4:sw=4:noet
#include "InterHub.h"
#include "Hub.h"
#include "Encoder.h"
#include "TigerHash.h"
#include "UserInfo.h"
#include "Plugin.h"

#include "error.h"
#include "Logs.h"
#include "Util.h"

using namespace std;
using namespace qhub;


class DNSLookup: public DNSAdapter {
public:
	DNSLookup(InterHub* ih, string s) : DNSAdapter(s), interHub(ih) {}
	virtual void complete(const string& r)
	{
		interHub->onLookup(r);
	}

protected:
	InterHub *interHub;
};


InterHub::InterHub(Hub* h, const string& hn, short p, const string& pa) throw()
		: ConnectionBase(h), hostname(hn), port(p), password(pa), outgoing(true)
{
	new DNSLookup(this, hn);
}

InterHub::InterHub(Hub* h, ADCSocket* s) throw()
		: ConnectionBase(h, s), outgoing(false)
{
}

void InterHub::onLookup(const string& ip)
{
	Logs::stat << "Connecting to hub at ip " << ip << " and port " << getPort() << endl;
	assert(getState() == PROTOCOL);

	getSocket()->connect(ip, getPort());
	doSupports();
	onConnected();
}

void InterHub::onConnected() throw()
{
	timeval tv;
	tv.tv_sec = 15;
	tv.tv_usec = 0;
	getSocket()->enableMe(EventHandler::ev_none, &tv);
	Plugin::InterConnected action;
	Plugin::fire(action, this);
}

void InterHub::onDisconnected(const string& clue) throw()
{
	if(getState() != NORMAL)
		return;
	getHub()->deactivate(this);
	Plugin::InterDisconnected action;
	Plugin::fire(action, this);
	delete this;	// hope this doesn't cause segfaults :)
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
	// get rid of timeout
	getSocket()->enableMe(EventHandler::ev_none, NULL);

	if(sl[0].size() != 4) {
		throw parse_error("command or CID invalid");
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
			throw command_error("bad command type in state PROTOCOL");
		}
		if (sl.size() < 4 || find(sl.begin()+2, sl.end(), "+BASE") == sl.end() ||
				find(sl.begin()+2, sl.end(), "+IHUB") == sl.end()) {
			throw command_error("invalid supports");
		}
		if(!outgoing) {
			doSupports();
			doAskPassword();
		}
		state = VERIFY;
		break;
	case VERIFY:
		if(outgoing) {
			if(command == (GPA | 'L')) {
				doPassword(sl);
			} else if(command == (STA | 'L') && sl.size() == 5
					&& sl[2] == "000" && sl[4] == "FCLPAS") {
				doInf();
				getHub()->activate(this);
				state = NORMAL;
			} else {
				throw command_error("bad command type; expected GPA or STA");
			}
		} else {
			if(command == (PAS | 'L')) {
				handlePassword(sl);
				send("LSTA " + getHub()->getCID32() + " 000 " + ADC::ESC("Password accepted.")
						+ " FCLPAS\n");
				doInf();
				getHub()->activate(this);
				state = NORMAL;
			} else {
				throw command_error("bad command type; expected PAS");
			}
		}
		break;
/*	case IDENTIFY:
		if(command != (INF | 'S')) {
			doError("bad command type");
			getSocket()->disconnect("InterHub::onLine bad command type in state IDENTIFY");
			return;
		}
		cid = sl[1];
		if(!ADC::checkCID(cid)) {
			getSocket()->disconnect("InterHub::onLine invalid CID: " + sl[1]);
			return;
		}
		//TODO: verify INF stuff
		doAskPassword();
		state = NORMAL;
		break;*/
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
	getHub()->getInterList(this);
	getHub()->getUserList(this);
}

void InterHub::doAskPassword() throw()
{
	assert(state == IDENTIFY && salt.empty() && !outgoing);
	salt = Util::genRand(24);
	send("LGPA " + getHub()->getCID32() + ' ' + Encoder::toBase32(&salt.front(), salt.size()) + '\n');
}

void InterHub::doPassword(const StringList& sl) throw()
{
	assert(state == VERIFY && outgoing);
	size_t len = sl[2].size() * 5 / 8;
	uint8_t* s = new uint8_t[len];
	Encoder::fromBase32(sl[2].data(), s, len);
	TigerHash h;
	h.update(getHub()->getCID32().data(), getHub()->getCID32().size());
	h.update(password.data(), password.size());
	h.update(s, len);
	delete[] s;
	send("LPAS " + getHub()->getCID32() + ' ' +
			Encoder::toBase32(h.finalize(), TigerHash::HASH_SIZE) + '\n');
}

void InterHub::handle(const StringList& sl, string& full, uint32_t command) throw(command_error)
{
	switch(full[0]) {
	case 'B':
		if(command == (INF | 'B')) {
			getHub()->addRemoteClient(sl[1], UserInfo(NULL, sl));
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
		if(command == (INF | 'S')) {
			getHub()->addRemoteHub(sl[1], UserInfo(NULL, sl), this);
		}
		break;
	case 'L':
		if(sl[1] != cid) {
			doError("CID mismatch");
			getSocket()->disconnect("CID mismatch");
		}
		break;
	case 'I':
		full.replace(5, 13, getHub()->getCID32());
		getHub()->broadcast(full, this);
		if(command == ('I' | QUI)) {
			getHub()->removeClient(sl[2]);
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

void InterHub::handlePassword(const StringList& sl) throw(command_error)
{
	TigerHash h;
	h.update(getCID32().data(), getCID32().size());
	h.update(getHub()->getInterPass().data(), getHub()->getInterPass().size());
	h.update(&salt.front(), salt.size());
	h.finalize();
	string result32 = Encoder::toBase32(h.getResult(), TigerHash::HASH_SIZE);
	if(result32 != sl[2]) {
		send("LSTA " + getHub()->getCID32() + " 223 " + ADC::ESC("Bad password") + '\n');
		state = PROTOCOL; // so it doesn't send error
		throw command_error("bad password: expected " + result32);
	}
	salt.clear();
}
