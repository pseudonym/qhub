// vim:ts=4:sw=4:noet
#include "Hub.h"

#include "qhub.h"
#include "ServerSocket.h"
#include "ADCClient.h"
#include "InterHub.h"
#include "Buffer.h"

#include <stdlib.h>
#include <adns.h>
#include <oop-adns.h>

using namespace qhub;

Hub::Hubs Hub::hubs;

Hub::Hub() : maxPacketSize(65536)
{
	add(this);
}

Hub::~Hub()
{
	remove(this);
}

void Hub::onLookup(adns_answer *reply) const
{
	fprintf(stderr, "Majs %s\n", reply->owner);
	string s(reply->owner);
	InterHub* ih = interConnects.find(s)->second;

	if (adns_s_ok != reply->status) {
		fprintf(stderr, " error: %s\n",adns_strerror(reply->status));
	} else {
		assert(adns_r_a == reply->type);
		if(reply->nrrs > 0){
			struct sockaddr_in dest_addr;
			dest_addr.sin_family = AF_INET;
			dest_addr.sin_port = htons(ih->getPort());
			dest_addr.sin_addr.s_addr = inet_addr(inet_ntoa(reply->rrs.inaddr[0]));
			memset(&(dest_addr.sin_zero), '\0', 8);

			::connect(ih->getFd(), (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
			enable_fd(ih->getFd(), OOP_READ, ih);
			ih->connect();
		}
	}
}

void Hub::openInterConnection(string host, int port, string password)
{
	//Do a DNS-lookup
	lookup(host.c_str(), this);
	InterHub* tmp = new InterHub(this);
	tmp->setHostName(host);
	tmp->setPort(port);
	tmp->setPassword(password);
	//no more than one connection per IP, sorry
	interConnects[host] = tmp;
}

void Hub::openADCPort(int port)
{
	//Leaf-handler
	ServerSocket* tmp = new ServerSocket(Socket::IP6, port, ServerSocket::LEAF_HANDLER, this);
	enable_fd(tmp->getFd(), OOP_READ, tmp);
}

void Hub::openInterPort(int port)
{
	//Inter-hub
	ServerSocket* tmp = new ServerSocket(Socket::IP6, port, ServerSocket::INTER_HUB, this);
	enable_fd(tmp->getFd(), OOP_READ, tmp);
}

void Hub::acceptLeaf(int fd, Socket::Domain d)
{
	Socket* tmp = new ADCClient(fd, d, this);
}

void Hub::getUserList(ADCClient* c) throw()
{
	string tmp;
	for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); i++) {
		tmp += i->second->getAdcInf();
	}
	for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); i++) {
		tmp += i->second->getAdcInf();
	}
	Buffer::writeBuffer t(new Buffer(tmp));
	c->writeb(t);
}

void Hub::motd(ADCClient* c) throw()
{
	char t[1024];
	sprintf(t, "%d", interConnects.size());
	char t2[1024];
	sprintf(t2, "%d", interConnects2.size());
	string tmp = string("There are ") + t + " hubs connected to us and " + t2 + " connected to from us.";
	sprintf(t2, "%dA+%dP", activeUsers.size(), passiveUsers.size());
	sprintf(t, "%d", 0);
	tmp += string("\nWe have ") + t2 + " local users, and " + t + " remote users.";
	c->doHubMessage(tmp);
}

void Hub::direct(string const& guid, string const& data, ADCClient* from) throw()
{
	Users::iterator i;
	if((i = activeUsers.find(guid)) != activeUsers.end() || (i = passiveUsers.find(guid)) != passiveUsers.end()) {
		Buffer::writeBuffer tmp(new Buffer(data, PRIO_NORM));
		if(from)
			from->writeb(tmp);
		i->second->writeb(tmp);
	} else {
		fprintf(stderr, "Send to non-existing user.\n");
	}
}

void Hub::broadcast(string const& data, ADCClient* except/* = NULL*/) throw()
{
	Buffer::writeBuffer tmp(new Buffer(data, PRIO_NORM));
	if(!except) {
		for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
			i->second->writeb(tmp);
		}
		for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i) {
			i->second->writeb(tmp);
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
	}
}

void Hub::broadcastActive(string const& data) throw()
{
	Buffer::writeBuffer tmp(new Buffer(data, PRIO_NORM));
	for(Users::iterator i = activeUsers.begin(); i != activeUsers.end(); ++i) {
		i->second->writeb(tmp);
	}
}

void Hub::broadcastPassive(string const& data) throw()
{
	Buffer::writeBuffer tmp(new Buffer(data, PRIO_NORM));
	for(Users::iterator i = passiveUsers.begin(); i != passiveUsers.end(); ++i) {
		i->second->writeb(tmp);
	}
}

void Hub::setMaxPacketSize(int s)
{
	maxPacketSize = s;
}

bool Hub::hasClient(string const& guid) const throw()
{
	if(activeUsers.find(guid) == activeUsers.end() && passiveUsers.find(guid) == passiveUsers.end())
		return false;
	return true;
}

void Hub::addActiveClient(string const& guid, ADCClient* client) throw()
{
	assert(!hasClient(guid));
	activeUsers[guid] = client;
}

void Hub::addPassiveClient(string const& guid, ADCClient* client) throw()
{
	assert(!hasClient(guid));
	passiveUsers[guid] = client;
}

void Hub::switchClientMode(bool toActive, string const& guid, ADCClient* client) throw()
{
	assert((!toActive && activeUsers.find(guid) != activeUsers.end()) ||
			(toActive && passiveUsers.find(guid) != passiveUsers.end()));
	if(!toActive) {
		activeUsers.erase(guid);
		passiveUsers[guid] = client;
	} else {
		passiveUsers.erase(guid);
		activeUsers[guid] = client;
	}
}

void Hub::removeClient(string const& guid) throw()
{
	assert(hasClient(guid));
	Users::iterator i = activeUsers.find(guid);
	if(i != activeUsers.end())
		activeUsers.erase(i);
	else
		passiveUsers.erase(guid);
}

ADCClient* Hub::getClient(string const& guid) throw()
{
	Users::iterator i;
	if((i = activeUsers.find(guid)) != activeUsers.end()) {
		return i->second;
	} else if((i = passiveUsers.find(guid)) != passiveUsers.end()) {
		return i->second;
	}
	return NULL;
}

void Hub::acceptInterHub(int fd, Socket::Domain d)
{
	InterHub* tmp = new InterHub(fd, d);
	interConnects2.push_back(tmp);
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
