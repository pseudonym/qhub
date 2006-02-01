// vim:ts=4:sw=4:noet
#include "Hub.h"

#include "qhub.h"
#include "error.h"
#include "ServerSocket.h"
#include "ADCClient.h"
#include "InterHub.h"
#include "Buffer.h"
#include "EventHandler.h"
#include "Logs.h"

#include <cstdlib>
#include <algorithm>
#include <iterator>
#include <boost/format.hpp>

using namespace qhub;

Hub::Hubs Hub::hubs;

Hub::Hub(const string& cid32_, const string& name_) : name(name_), cid32(cid32_)
{
	add(this);
}

Hub::~Hub()
{
	remove(this);
}

void Hub::openClientPort(int port)
{
	//Leaf-handler
	ServerSocket* tmp = NULL;
#ifdef ENABLE_IPV6
	try {
		tmp = new ServerSocket(Socket::IP6, port, ServerSocket::LEAF_HANDLER, this);
		return;
	} catch(const socket_error&) {
		delete tmp;
		// oh well, just try using IPv4...
	}
#endif

	try {
		tmp = new ServerSocket(Socket::IP4, port, ServerSocket::LEAF_HANDLER, this);
		return;
	} catch(const socket_error& e) {
		delete tmp;
		Logs::err << "opening client port " << port << "failed: " << e.what() << endl;
	}
}

void Hub::openInterPort(int port)
{
	//Inter-hub
	ServerSocket* tmp = NULL;
#ifdef ENABLE_IPV6
	try {
		tmp = new ServerSocket(Socket::IP6, port, ServerSocket::INTER_HUB, this);
		tmp->enableMe(EventHandler::ev_read);
		return;
	} catch(const socket_error&) {
		delete tmp;
	}
#endif

	try {
		tmp = new ServerSocket(Socket::IP4, port, ServerSocket::INTER_HUB, this);
		tmp->enableMe(EventHandler::ev_read);
		return;
	} catch(const socket_error& e) {
		delete tmp;
		Logs::err << "opening server port " << port << "failed: " << e.what() << endl;
	}
}

void Hub::acceptLeaf(int fd, Socket::Domain d)
{
	// looks odd, but does what it's supposed to
	new ADCClient(this, new ADCSocket(fd, d));
}

void Hub::getUserList(ConnectionBase* c) throw()
{
	Buffer::Ptr t(new Buffer(Util::emptyString));
	string& tmp = t->getBuf();
	for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); i++) {
		tmp += i->second->getAdcInf();
	}
	for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); i++) {
		tmp += i->second->getAdcInf();
	}
	for(RemoteUsers::iterator i = remoteUsers.begin(); i != remoteUsers.end(); i++) {
		tmp += i->second.toADC(i->first);
	}
	c->getSocket()->writeb(t);
}

void Hub::getInterList(InterHub* ih) throw()
{
	for(RemoteHubs::iterator i = remoteHubs.begin(); i != remoteHubs.end(); ++i) {
		ih->send(i->second->getUserInfo()->toADC(i->first));
	}
}

void Hub::motd(ADCClient* c) throw()
{
	boost::format f("Hubconnections: %d.\nWe have %d (of which %d are passive) local users, and %d remote users.");
	f % interhubs.size() % (activeUsers.size()+passiveUsers.size()) % passiveUsers.size() % remoteUsers.size();
	c->doHubMessage(f.str());
}

void Hub::direct(string const& cid, string const& data, Client* from) throw()
{
	Users::iterator i;
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	if((i = activeUsers.find(cid)) != activeUsers.end() || (i = passiveUsers.find(cid)) != passiveUsers.end()) {
		if(from)
			from->getSocket()->writeb(tmp);
		i->second->getSocket()->writeb(tmp);
	} else {
		RemoteUsers::iterator j = remoteUsers.find(cid);
		if(j == remoteUsers.end())
			return;
		RemoteHubs::iterator k = remoteHubs.find(j->second.getRealHub());
		if(k == remoteHubs.end())
			return;
		k->second->getInterHub()->send(data);  // wow. ugly as sin
	}
}

void Hub::broadcast(string const& data, ConnectionBase* except/* = NULL*/) throw()
{
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	broadcastInter(data, dynamic_cast<InterHub*>(except));
	for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
		if(i->second != except)
			i->second->getSocket()->writeb(tmp);
	}
	for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i) {
		if(i->second != except)
			i->second->getSocket()->writeb(tmp);
	}
}

void Hub::broadcastActive(string const& data, ConnectionBase* except/* = NULL*/) throw()
{
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	broadcastInter(data, dynamic_cast<InterHub*>(except));
	for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
		if(i->second != except)
			i->second->getSocket()->writeb(tmp);
	}
}

void Hub::broadcastPassive(string const& data, ConnectionBase* except/* = NULL*/) throw()
{
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	broadcastInter(data, dynamic_cast<InterHub*>(except));
	for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i) {
		if(i->second != except)
			i->second->getSocket()->writeb(tmp);
	}
}

void Hub::broadcastInter(string const& data, InterHub* except/* = NULL*/) throw()
{
	for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i)
		if(*i != except)
			(*i)->send(data);
}

void Hub::broadcastFeature(string const& data, const string& feat, bool yes, ConnectionBase* except/* = NULL*/) throw()
{
	broadcastInter(data, dynamic_cast<InterHub*>(except)); // NULL on error, what we want...
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
		if(i->second->getUserInfo()->hasSupport(feat) == yes)
			i->second->getSocket()->writeb(tmp);
	}
	for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i) {
		if(i->second->getUserInfo()->hasSupport(feat) == yes)
			i->second->getSocket()->writeb(tmp);
	}
}

bool Hub::hasClient(string const& cid, bool localonly) const throw()
{
	if(activeUsers.count(cid))
		return true;
	if(passiveUsers.count(cid))
		return true;
	if(!localonly && remoteUsers.count(cid))
		return true;
	if(!localonly && remoteHubs.count(cid))
		return true;
	return false;
}

void Hub::addActiveClient(string const& cid, ADCClient* client) throw()
{
	assert(!hasClient(cid));
	activeUsers[cid] = client;
}

void Hub::addPassiveClient(string const& cid, ADCClient* client) throw()
{
	assert(!hasClient(cid));
	passiveUsers[cid] = client;
}

void Hub::addRemoteClient(string const& cid, UserInfo const& ui) throw()
{
	assert(!hasClient(cid) || remoteUsers.count(cid));
	remoteUsers[cid].update(ui);
	assert(ADC::checkCID(remoteUsers[cid].get("CH")));
}

void Hub::switchClientMode(bool toActive, string const& cid, ADCClient* client) throw()
{
	assert((!toActive && activeUsers.count(cid)) ||
	       (toActive && passiveUsers.count(cid)));
	if(!toActive) {
		activeUsers.erase(cid);
		passiveUsers[cid] = client;
	} else {
		passiveUsers.erase(cid);
		activeUsers[cid] = client;
	}
}

void Hub::removeClient(string const& cid) throw()
{
	assert(hasClient(cid));
	if(activeUsers.count(cid))
		activeUsers.erase(cid);
	else if(passiveUsers.count(cid))
		passiveUsers.erase(cid);
	else
		remoteUsers.erase(cid);
}

void Hub::addRemoteHub(string const& cid, const UserInfo& ui, InterHub* conn) throw()
{
	if(remoteHubs.count(cid)) {
		assert(conn == remoteHubs[cid]->getInterHub());
		remoteHubs[cid]->getUserInfo()->update(ui);
	} else {
		remoteHubs.insert(make_pair(cid, new RemoteHub(cid, ui, conn)));
	}
}

void Hub::removeRemoteHub(string const& cid) throw()
{
	for(RemoteUsers::iterator i = remoteUsers.begin(); i != remoteUsers.end(); ) {
		if(i->second.getRealHub() == cid) {
			broadcast("IQUI " + getCID32() + ' ' + i->first + '\n');
			remoteUsers.erase(i++);
		} else {
			++i;
		}
	}
	RemoteHubs::iterator i = remoteHubs.find(cid);
	if(i != remoteHubs.end()) {
		delete i->second;
		remoteHubs.erase(i);
	}
}

void Hub::userDisconnect(string const& actor, string const& victim, string const& msg) throw()
{
	Users::iterator i;
	if((i = activeUsers.find(victim)) != activeUsers.end())
		i->second->doDisconnectBy(actor, msg);
	else if((i = passiveUsers.find(victim)) != passiveUsers.end())
		i->second->doDisconnectBy(actor, msg);
}

void Hub::openInterConnection(const string& host, int port, const string& pass) throw()
{
	//we don't want this added anywhere until it's functional
	//it will add itself once it's ready to carry traffic
	new InterHub(this, host, (short)port, pass);
}

void Hub::acceptInterHub(int fd, Socket::Domain d)
{
	//see comment above
	new InterHub(this, new ADCSocket(fd, d));
}

bool Hub::hasNick(const string& nick) throw()
{
	// O(n)... not very pretty
	for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i)
		if(nick == i->second->getUserInfo()->getNick())
			return true;
	for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i)
		if(nick == i->second->getUserInfo()->getNick())
			return true;
	for(RemoteUsers::iterator i = remoteUsers.begin(); i != remoteUsers.end(); ++i)
		if(nick == i->second.getNick())
			return true;
	return false;
}

void Hub::activate(InterHub* ih) throw()
{
	interhubs.push_back(ih);
}

void Hub::deactivate(InterHub* ih) throw()
{
	Interhubs::iterator i = find(interhubs.begin(), interhubs.end(), ih);
	if(i != interhubs.end())
		interhubs.erase(i);
}

void Hub::killAll() throw()
{
	while(!hubs.empty())
		delete hubs.back();
}

void Hub::add(Hub* h) throw()
{
	assert(find(hubs.begin(), hubs.end(), h) == hubs.end());
	hubs.push_back(h);
}

void Hub::remove(Hub* h) throw()
{
	assert(find(hubs.begin(), hubs.end(), h) != hubs.end());
	hubs.erase(find(hubs.begin(), hubs.end(), h));
}
