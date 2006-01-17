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
		tmp->enableMe(EventHandler::ev_read);
		return;
	} catch(const socket_error&) {
		delete tmp;
		// oh well, just try using IPv4...
	}
#endif
	try {
		tmp = new ServerSocket(Socket::IP4, port, ServerSocket::LEAF_HANDLER, this);
		tmp->enableMe(EventHandler::ev_read);
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
	new ADCClient(fd, d, this);
}

void Hub::getUserList(ADCSocket* c) throw()
{
	Buffer::Ptr t(new Buffer(Util::emptyString));
	string& tmp = t->getBuf();
	for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); i++) {
		tmp += i->second->getAdcInf();
	}
	for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); i++) {
		tmp += i->second->getAdcInf();
	}
	for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); i++) {
		(*i)->appendUserList(tmp);
	}
	c->writeb(t);
}

inline size_t Hub::motdHelper()
{
	size_t sum = 0;
	for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i)
		sum += (*i)->getNumUsers();
	return sum;
}

void Hub::motd(ADCClient* c) throw()
{
	boost::format fmt("Hubconnections: %d.\nWe have %d (of which %d are passive) local users, and %d remote users.");
	fmt % interhubs.size() % (activeUsers.size()+passiveUsers.size()) % passiveUsers.size() % motdHelper();
	c->doHubMessage(fmt.str());
}

void Hub::direct(string const& cid, string const& data, ADCClient* from) throw()
{
	Users::iterator i;
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	if((i = activeUsers.find(cid)) != activeUsers.end() || (i = passiveUsers.find(cid)) != passiveUsers.end()) {
		if(from)
			from->writeb(tmp);
		i->second->writeb(tmp);
	} else {
		//send it along to the correct connected hub
		for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i)
			if((*i)->hasClient(cid))
				(*i)->writeb(tmp);
	}
}

void Hub::broadcast(string const& data, ADCSocket* except/* = NULL*/) throw()
{
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	if(!except) {
		for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
			i->second->writeb(tmp);
		}
		for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i) {
			i->second->writeb(tmp);
		}
		for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i) {
			(*i)->writeb(tmp);
		}
	} else {
		for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
			if(i->second != except)
				i->second->writeb(tmp);
		}
		for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i) {
			if(i->second != except)
				i->second->writeb(tmp);
		}
		for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i) {
			if(*i != except)
				(*i)->writeb(tmp);
		}
	}
}

void Hub::broadcastActive(string const& data, ADCSocket* except/* = NULL*/) throw()
{
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	if(!except) {
		for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
			i->second->writeb(tmp);
		}
		for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i) {
			(*i)->writeb(tmp);
		}
	} else {
		for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
			if(i->second != except)
				i->second->writeb(tmp);
		}
		for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i) {
			if(*i != except)
				(*i)->writeb(tmp);
		}
	}
}

void Hub::broadcastPassive(string const& data, ADCSocket* except/* = NULL*/) throw()
{
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	if(!except) {
		for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i) {
			i->second->writeb(tmp);
		}
		for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i) {
			(*i)->writeb(tmp);
		}
	} else {
		for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i) {
			if(i->second != except)
				i->second->writeb(tmp);
		}
		for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i) {
			if(*i != except)
				(*i)->writeb(tmp);
		}
	}
}

void Hub::broadcastInter(string const& data, InterHub* except/* = NULL*/) throw()
{
	for(Interhubs::iterator i = interhubs.begin(); i != interhubs.end(); ++i) {
		if(*i != except)
			(*i)->send(data);
	}
}

void Hub::broadcastFeature(string const& data, const string& feat, bool yes, ADCSocket* except/* = NULL*/) throw()
{
	broadcastInter(data, dynamic_cast<InterHub*>(except)); // NULL on error, what we want...
	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
		if(i->second->getUserInfo()->hasSupport(feat) == yes)
			i->second->writeb(tmp);
	}
}

bool Hub::hasClient(string const& cid, bool localonly) const throw()
{
	if(activeUsers.find(cid) != activeUsers.end())
		return true;
	if(passiveUsers.find(cid) != passiveUsers.end())
		return true;
	if(!localonly)
		for(Interhubs::const_iterator i = interhubs.begin(); i != interhubs.end(); ++i)
			if((*i)->hasClient(cid))
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
	else
		passiveUsers.erase(cid);
}

void Hub::userDisconnect(string const& actor, string const& victim, string const& msg) throw()
{
	Users::iterator i;
	if((i = activeUsers.find(victim)) != activeUsers.end())
		i->second->doDisconnectBy(actor, msg);
	else if((i = passiveUsers.find(victim)) != passiveUsers.end())
		i->second->doDisconnectBy(actor, msg);
}

void Hub::openInterConnection(const string& host, int port) throw()
{
	//we don't want this added anywhere until it's functional
	//it will add itself once it's ready to carry traffic
	new InterHub(this, host, (short)port);
}

void Hub::acceptInterHub(int fd, Socket::Domain d)
{
	//see comment above
	new InterHub(this, fd, d);
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
		delete *hubs.begin();
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
