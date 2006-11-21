// vim:ts=4:sw=4:noet
#include "Client.h"
#include "Hub.h"
#include "TigerHash.h"
#include "Encoder.h"
#include "Plugin.h"
#include "UserInfo.h"
#include "UserData.h"
#include "ADC.h"
#include "Logs.h"
#include "ClientManager.h"
#include "ServerManager.h"
#include "PluginManager.h"

using namespace std;
using namespace qhub;

Client::Client(ADCSocket* s) throw()
		: ConnectionBase(s), added(false), userData(NULL), userInfo(NULL)
{
	onConnected();
}

Client::~Client() throw()
{
	if(userInfo)
		delete userInfo;
	if(userData)
		delete userData;
}

UserData* Client::getUserData() throw()
{
	if(!userData)
		userData = new UserData;
	return userData;
}

void Client::login() throw()
{
	// Stop alarm. Else we'd get booted.
	EventManager::instance()->removeTimer(getSocket());

	state = NORMAL;
	Plugin::UserConnected action;
	PluginManager::instance()->fire(action, this);
	if(action.isSet(Plugin::DISCONNECTED) || action.isSet(Plugin::HANDLED))
		return;

	// send INF of hub bot to ops
	if(userInfo->getOp())
		send(Command('B', Command::INF, Hub::instance()->getBotSid())
				<< "NIqhub" << "BO1" << "DEBOT -- pm to control hub"
				<< "VE" PACKAGE_NAME "/" PACKAGE_VERSION
				<< "HO1" << "OP1"
				<< "IDTHISISTHECIDOFTHEQHUBBOTAAAAAAAAAAAAAAA");
	// Send INFs
	ClientManager::instance()->getUserList(this);

	added = true;
	ClientManager::instance()->addLocalClient(getSid(), this);
	// Notify him that userlist is over and notify others of his presence
	dispatch(getAdcInf());
	Hub::instance()->motd(this);
}

void Client::logout() throw()
{
	ClientManager::instance()->removeClient(getSid());
	added = false;
	Plugin::UserDisconnected action;
	PluginManager::instance()->fire(action, this);
}

Command const& Client::getAdcInf() throw()
{
	return userInfo->toADC(getSid());
}


/*******************************************/
/* Calls from ADCSocket (and other places) */
/*******************************************/

void Client::doAskPassword(string const& pwd) throw()
{
	assert(state == IDENTIFY && !added);
	password = pwd;
	salt = Util::genRand(24);
	send(Command('I', Command::GPA) << Encoder::toBase32(&salt.front(), salt.size()));
	state = VERIFY;
}

void Client::doWarning(string const& msg) throw()
{
	send(Command('I', Command::STA) << "100" << msg);
}

void Client::doError(string const& msg, int code, string const& flag) throw()
{
	Command cmd('I', Command::STA);
	cmd << (format("2%02d") % code).str() << msg;
	if(!flag.empty())
		cmd << flag;
	send(cmd);
}

void Client::doDisconnect(string const& msg) throw()
{
	if(added) {
		logout();
		if(msg.empty())
			dispatch(Command('I', Command::QUI)
					<< ADC::fromSid(getSid()));
		else
			dispatch(Command('I', Command::QUI)
					<< ADC::fromSid(getSid()) << CmdParam("MS", msg));
	}
	getSocket()->disconnect(msg);
}

void Client::doHubMessage(string const& msg) throw()
{
	send(Command('I', Command::MSG) << msg);
}

void Client::doPrivateMessage(string const& msg) throw()
{
	send(Command(Command::MSG, Hub::instance()->getBotSid(), getSid()) << msg
			<< CmdParam("PM", ADC::fromSid(Hub::instance()->getBotSid())));
}

void Client::doDisconnectBy(sid_type kicker, string const& msg) throw()
{
	dispatch(Command('I', Command::QUI)
			<< ADC::fromSid(getSid())
			<< CmdParam("ID", ADC::fromSid(kicker))
			<< CmdParam("MS", msg));
	logout();
	getSocket()->disconnect();
}

#define PROTOCOL_ERROR(errmsg) throw command_error(errmsg)

void Client::onLine(Command& cmd) throw(command_error)
{
	// Plugin fire ClientLine
	{
		Plugin::ClientLine action;
		PluginManager::instance()->fire(action, this, cmd);
		if(action.isSet(Plugin::DISCONNECTED) || action.isSet(Plugin::STOPPED))
			return;
	}

	// Do specialized input checking
	switch(cmd.getAction()) {
	case 'H':
		// ? do we wish supports to happen just whenever or at PROTOCOL only ?
		if(state == PROTOCOL && cmd.getCmd() == Command::SUP) {
			handleSupports(cmd);
			return;
		// ? same with HPAS ?
		} else if(state == VERIFY && cmd.getCmd() == Command::PAS) {
			handlePassword(cmd);
			return;
		}
		break;
	case 'B':
		if(state == IDENTIFY && cmd.getCmd() == Command::INF) {
			handleLogin(cmd);
			return;
		}
		break;
	default:
		break;
	}

	// All non-NORMAL states have been handled
	if(state != NORMAL) {
		PROTOCOL_ERROR("State mismatch: NORMAL expected");
	}

	// make sure they send the correct SID (if they send one)...
	if(cmd.getSource() != getSid() && cmd.getSource() != INVALID_SID) {
		PROTOCOL_ERROR("SID mismatch: " + ADC::fromSid(getSid()) + " expected");
	}

	// Check message type
	switch(cmd.getAction()) {
	case 'B':
	case 'D':
	case 'H':
	case 'F':
		break;
	case 'C':
	case 'I':
	case 'U':
	default:
		PROTOCOL_ERROR(string("Message type unsupported: ") + (char)(cmd.getAction() & 0x000000FF) + " recieved");
		return;
	}

	// Woohoo! A normal message to process
	handle(cmd);
}

void Client::onConnected() throw()
{
	// disconnect if they do not complete login after 15 seconds
	EventManager::instance()->addTimer(getSocket(), 0, 15);

	Plugin::ClientConnected action;
	PluginManager::instance()->fire(action, this);
}

void Client::onDisconnected(string const& clue) throw()
{
	if(added) {
		// this is here so ADCSocket can safely destroy us.
		// if we don't want a second message and our victim to get the message as well
		// remove us when doing e.g. the Kick, so that added is false here.
		Logs::stat << format("onDisconnected %d %p SID: %s")
				% getSocket()->getFd() % this % ADC::fromSid(getSid()) << endl;
		logout();
		if(clue.empty())
			dispatch(Command('I', Command::QUI)
					<< ADC::fromSid(getSid()));
		else
			dispatch(Command('I', Command::QUI)
					<< ADC::fromSid(getSid()) << CmdParam("MS", clue));
	}
	Plugin::ClientDisconnected action;
	PluginManager::instance()->fire(action, this);
	delete this;	// hope this doesn't cause segfaults :)
}



/*****************/
/* Data handlers */
/*****************/

void Client::handle(Command& cmd) throw(command_error) {
	// Check if we need to handle anything, if not, do default action.

	// * HDSC *
	if(cmd.getAction() == 'H') {
		if(cmd.getCmd() == Command::DSC) {
			handleDisconnect(cmd);
			return;
		} else {
			doWarning("Unknown hub-directed message ignored");
			return;
		}
	// * BINF *
	} else if(cmd.getCmd() == Command::INF) {
		if(cmd.getAction() == 'B') {
			handleInfo(cmd);
			return;
		} else {
			doWarning("INF message type invalid");
			return;
		}
	// * ?MSG *
	} else if(cmd.getCmd() == Command::MSG) {
		handleMessage(cmd);
		return;
	// * Everything else *
	} else {
		dispatch(cmd);
	}
}

void Client::handleLogin(Command& cmd) throw(command_error)
{
	assert(state == IDENTIFY);

	/*if(ClientManager::instance()->hasClient(cid) || cid == getHub()->getCID32()) {
		// Ping other user, perhaps it's a ghost
		getHub()->direct(cid, "\n");
		PROTOCOL_ERROR("CID busy, change CID or wait");
		// Note: Don't forget to check again at HPAS.. perhaps someone beat us to it.
	}*/

	// Load info
	userInfo = new UserInfo(cmd);
	userInfo->del("OP"); //can't have them opping themselves...

	// Guarantee NI and (I4 nand I6)
	if(!userInfo->has("NI") || (userInfo->has("I4") && userInfo->has("I6")))
		throw command_error("Missing/extraneous parameters in INF");

	// Fix I4 and I6
	if(userInfo->has("I4") && userInfo->get("I4") == "0.0.0.0") {
		if(getSocket()->getDomain() == Socket::IP4)
			userInfo->set("I4", getSocket()->getPeerName());
		else
			throw command_error("'I4' sent in INF but not connected via IPv4");
	}
#ifdef ENABLE_IPV6
	else if(userInfo->has("I6") && userInfo->get("I6") == "[0:0:0:0:0:0:0:0]") {
		if(getSocket()->getDomain() == Socket::IP6)
			userInfo->set("I6", getSocket()->getPeerName());
		else
			throw command_error("'I6' sent in INF but not connected via IPv6");
	}
#endif
	const string& pid32 = userInfo->get("PD");
	const string& cid32 = userInfo->get("ID");
	if(pid32.size() != 39) // 39 = ceil(24*8/5)
		throw command_error("PID missing/invalid");
	if(cid32.size() != 39)
		throw command_error("CID missing/invalid");
	uint8_t* pid = new uint8_t[TigerHash::HASH_SIZE];
	Encoder::fromBase32(pid32.data(), pid, TigerHash::HASH_SIZE);
	TigerHash th;
	th.update(pid, TigerHash::HASH_SIZE);
	delete[] pid;
	th.finalize();
	if(cid32 != Encoder::toBase32(th.getResult(), TigerHash::HASH_SIZE))
		throw command_error("CID/PID mismatch");
	userInfo->del("PD");

	if(ClientManager::instance()->hasCid(userInfo->getCID()))
		throw command_error("CID taken");
	if(ClientManager::instance()->hasNick(userInfo->getNick())) {
		string newNick = "guest" + ADC::fromSid(getSid());
		if(ClientManager::instance()->hasNick(newNick)) {
			throw command_error("nick taken, and fallback nick invalid");
		} else {
			doWarning("Nick taken, using nick " + newNick);
			userInfo->set("NI", newNick);
		}
	}

	// Broadcast
	Plugin::ClientLogin action;
	PluginManager::instance()->fire(action, this);
	if(action.isSet(Plugin::DISCONNECTED))
		return;

	if(password.empty())
		login();
}

void Client::handleInfo(Command& cmd) throw(command_error)
{
	assert(state == NORMAL);

	UserInfo newUserInfo(cmd);

	Plugin::ClientInfo action;
	PluginManager::instance()->fire(action, this, newUserInfo);
	if(action.isSet(Plugin::DISCONNECTED) || action.isSet(Plugin::STOPPED))
		return;

	// Do redundancy check
	for(UserInfo::const_iterator i = newUserInfo.begin(); i != newUserInfo.end(); ++i) {
		if(userInfo->get(i->first) == i->second) {
			PROTOCOL_ERROR(string("Redundant INF parameter recieved: ")
					+ (char)(i->first >> 8) + (char)(i->first & 0xFF) + i->second);
			return;
		}
	}

	// no changing CIDs on us!
	newUserInfo.del("ID");

	// check nick; we know the current one is valid
	if(ClientManager::instance()->hasNick(newUserInfo.getNick())) {
		doWarning("That nick is already taken");
		newUserInfo.del("NI");
	}
	// TODO update nick list in ClientManager for dupes

	// Broadcast
	if(action.isSet(Plugin::MODIFIED)) {
		dispatch(newUserInfo.toADC(getSid()));
	} else {
		dispatch(cmd);
	}

	// Merge new data
	userInfo->update(newUserInfo);
}

void Client::handleMessage(Command& cmd) throw()
{
	if(cmd.getAction() == 'D' && cmd.getDest() == Hub::instance()->getBotSid()) {
		Plugin::UserCommand action;
		PluginManager::instance()->fire(action, this, cmd[0]);
		if(action.isSet(Plugin::DISCONNECTED))
			return;
		if(!action.isSet(Plugin::STOPPED))
			send(cmd);
	} else if(cmd.find("PM") == cmd.end()) {
		Plugin::UserMessage action;
		PluginManager::instance()->fire(action, this, cmd, cmd[0]);
		if(action.isSet(Plugin::DISCONNECTED))
			return;
		if(!action.isSet(Plugin::STOPPED))
			dispatch(cmd);
	} else {
		Plugin::UserPrivateMessage action;
		sid_type sid = ADC::toSid(cmd.find("PM")->substr(2));
		PluginManager::instance()->fire(action, this, cmd, cmd[0], sid);
		if(action.isSet(Plugin::DISCONNECTED))
			return;
		if(!action.isSet(Plugin::STOPPED))
			dispatch(cmd);
	}
}

void Client::handleDisconnect(Command& cmd) throw()
{
	if(!userInfo->getOp()) {
		send(Command('I', Command::STA) << "125" << "Access denied" << CmdParam("FC", "HDSC"));
		return;
	}
	// TODO add plugin stuff
	//getHub()->userDisconnect(cmd.getSource(), cmd.getDest(), Util::emptyString);
}

void Client::handlePassword(Command& cmd) throw()
{
	// Make hash
	TigerHash h;
	uint8_t buf[TigerHash::HASH_SIZE];
	Encoder::fromBase32(userInfo->get("ID").data(), buf, TigerHash::HASH_SIZE);
	h.update(buf, TigerHash::HASH_SIZE);
	h.update(password.data(), password.length());
	h.update(&salt.front(), salt.size());
	h.finalize();
	if(Encoder::toBase32(h.getResult(), TigerHash::HASH_SIZE) != cmd[0]) {
		send(Command('I', Command::STA) << "223" << "Bad username or password");
		assert(!added);
		doDisconnect("bad nick/pass");
		return;
	}
	salt.clear();
	// Add user
	/*if(getHub()->hasClient(getCID32())) {
		// TODO disconnect other user, probably a ghost
		send("ISTA 224 " + ADC::ESC("CID taken") + '\n');
		assert(!added);
		doDisconnect("CID taken");
		return;
	}*/
	// Oki, do login
	password.clear();
	login();
}

void Client::handleSupports(Command& cmd) throw(command_error)
{
	//uncomment this once DC++ follows the spec and doesn't send +BAS0
	/*if(find(sl.begin("AD"), sl.end("AD"), "BASE") == sl.end()) {
		PROTOCOL_ERROR("Invalid supports");
	}*/
	updateSupports(cmd);
	send(Command('I', Command::SUP) << CmdParam("AD", "BASE"));
	// use file descriptor for ID... just make sure we haven't overflowed
	if((unsigned)getSocket()->getFd() != (ServerManager::instance()->getClientSidMask() & getSocket()->getFd()))
		throw command_error("hub is full", 11);
	this->sid = (Hub::instance()->getSid() & ServerManager::instance()->getHubSidMask()) | getSocket()->getFd();
	send(Command('I', Command::SID) << ADC::fromSid(this->sid));
	send(Hub::instance()->getAdcInf());
	state = IDENTIFY;
}
