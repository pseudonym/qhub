// vim:ts=4:sw=4:noet
#include "InterHub.h"
#include "Hub.h"
#include "ClientManager.h"
#include "ServerManager.h"
#include "PluginManager.h"
#include "Encoder.h"
#include "TigerHash.h"
#include "UserInfo.h"
#include "Plugin.h"
#include "DNSAdapter.h"

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


InterHub::InterHub(const string& hn, short p, const string& pa) throw()
		: hostname(hn), port(p), password(pa), outgoing(true)
{
	new DNSLookup(this, hn);
}

InterHub::InterHub(ADCSocket* s) throw()
		: ConnectionBase(s), outgoing(false)
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
	// FIXME: ugly, should have general timer interface here
	int t = getSocket()->getEnabledFlags();
	getSocket()->disableMe(EventHandler::READ);
	getSocket()->disableMe(EventHandler::WRITE);
	getSocket()->enableMe((EventHandler::type)t, 15);

	Plugin::InterConnected action;
	PluginManager::instance()->fire(action, this);
}

void InterHub::onDisconnected(const string& clue) throw()
{
	ServerManager::instance()->deactivate(this);
	Plugin::InterDisconnected action;
	PluginManager::instance()->fire(action, this);
	delete this;	// hope this doesn't cause segfaults :)
}

void InterHub::doError(string const& msg, int code, string const& flag) throw()
{
	Command cmd('L', Command::STA);
	cmd << (format("2%02d") % code).str() << msg;
	if(!flag.empty())
		cmd << flag;
	send(cmd);
}

void InterHub::doWarning(const string& msg) throw()
{
	if(state == PROTOCOL)
		return;
	send(Command('L', Command::STA) << "100" << msg);
}

void InterHub::onLine(Command& cmd) throw(command_error)
{
	// get rid of timeout FIXME: see above
	int t = getSocket()->getEnabledFlags();
	getSocket()->disableMe(EventHandler::READ);
	getSocket()->disableMe(EventHandler::WRITE);
	getSocket()->enableMe((EventHandler::type)t);

	{
		Plugin::InterLine action;
		PluginManager::instance()->fire(action, this, cmd);
		if(action.isSet(Plugin::DISCONNECTED) || action.isSet(Plugin::STOPPED))
			return;
	}

	switch(state) {
	case PROTOCOL:
		if(cmd != (Command::SUP | 'L')) {
			throw command_error("bad command type in state PROTOCOL");
		}
		if(find(cmd.begin(), cmd.end(), "ADBASE") == cmd.end()
				|| find(cmd.begin(), cmd.end(), "ADIHUB") == cmd.end()) {
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
			if(cmd == (Command::GPA | 'L')) {
				doPassword(cmd);
				doInf();
				ServerManager::instance()->activate(this);
				state = INFLIST;
			} else {
				throw command_error("bad command type; expected GPA");
			}
		} else {
			if(cmd == (Command::PAS | 'L')) {
				handlePassword(cmd);
				doInf();
				ServerManager::instance()->activate(this);
				state = INFLIST;
			} else {
				throw command_error("bad command type; expected PAS");
			}
		}
		break;
	case INFLIST:
		// TODO: some sort of compression of these large INF lists (ZLIF)
		if(cmd != (Command::INF | 'B') && cmd != (Command::INF | 'S'))
			state = NORMAL;
	case NORMAL:
		//pass the message on
		handle(cmd);
		break;
	default:
		assert(0);
	}
}

void InterHub::doSupports() throw()
{
	send(Command('L', Command::SUP) << "ADBASE" << "ADIHUB");
}

void InterHub::doInf() throw()
{
	send(Command('S', Command::INF, Hub::instance()->getSid())
			<< CmdParam("HU", "1")
			<< CmdParam("NI", Hub::instance()->getName())
			<< CmdParam("DE", Hub::instance()->getDescription())
			<< CmdParam("VE", PACKAGE_NAME "/" PACKAGE_VERSION));
	ServerManager::instance()->getInterList(this);
	ClientManager::instance()->getUserList(this);
}

void InterHub::doAskPassword() throw()
{
	assert(state == IDENTIFY && salt.empty() && !outgoing);
	salt = Util::genRand(24);
	send(Command('L', Command::GPA) << Encoder::toBase32(&salt.front(), salt.size()));
}

void InterHub::doPassword(const Command& cmd) throw()
{
	assert(state == VERIFY && outgoing);
	size_t len = cmd[0].size() * 5 / 8;
	uint8_t* s = new uint8_t[len];
	Encoder::fromBase32(cmd[0].data(), s, len);
	TigerHash h;
	//h.update(getHub()->getCID32().data(), getHub()->getCID32().size());
	h.update(password.data(), password.size());
	h.update(s, len);
	delete[] s;
	send(Command('L', Command::PAS) << Encoder::toBase32(h.finalize(), TigerHash::HASH_SIZE));
}

void InterHub::handle(const Command& cmd) throw(command_error)
{
	if(cmd == (Command::INF | 'B')) {
		ClientManager::instance()->addRemoteClient(cmd.getSource(), UserInfo(cmd));
	}
	if(cmd == (Command::INF | 'S')) {
		ServerManager::instance()->add(cmd.getSource(), UserInfo(cmd), this);
	}
	if(cmd == ('I' | Command::QUI)) {
		ClientManager::instance()->removeClient(ADC::toSid(cmd[0]));
	}
	dispatch(cmd);
}

void InterHub::handlePassword(const Command& cmd) throw(command_error)
{
	TigerHash h;
	//h.update(getCID32().data(), getCID32().size());
	h.update(Hub::instance()->getInterPass().data(), Hub::instance()->getInterPass().size());
	h.update(&salt.front(), salt.size());
	h.finalize();
	const string& result32 = Encoder::toBase32(h.getResult(), TigerHash::HASH_SIZE);
	if(result32 != cmd[0]) {
		send(Command('L', Command::STA) << "223" << "Bad password");
		state = PROTOCOL; // so it doesn't send error
		throw command_error("bad password: expected " + result32);
	}
	salt.clear();
}
