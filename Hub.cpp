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
			enable(ih->getFd(), OOP_READ, ih);
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
	enable(tmp->getFd(), OOP_READ, tmp);
}

void Hub::openInterPort(int port)
{
	//Inter-hub
	ServerSocket* tmp = new ServerSocket(Socket::IP6, port, ServerSocket::INTER_HUB, this);
	enable(tmp->getFd(), OOP_READ, tmp);
}

void Hub::acceptLeaf(int fd, Socket::Domain d)
{
	Socket* tmp = new ADCClient(fd, d, this);
}

void Hub::getUsersList(ADCClient* c)
{
	string tmp;
	for(userIter i=users.begin(); i!=users.end(); i++){
		tmp += i->second->getInf();
	}

	Buffer::writeBuffer t(new Buffer(tmp));
	c->writeb(t);
}

void Hub::motd(ADCClient* c)
{
	char t[1024];
	sprintf(t, "%d", interConnects.size());
	char t2[1024];
	sprintf(t2, "%d", interConnects2.size());
	string tmp = string("There are ") + t + " hubs connected to us and " + t2 + " connected to from us.";
	sprintf(t2, "%d", users.size());
	sprintf(t, "%d", 0);
	tmp += string("\nWe have ") + t2 + " local users, and " + t + " remote users.";
	c->doHubMessage(tmp);
}

void Hub::direct(string guid, string data)
{
	userIter i = users.find(guid);
	if(i != users.end()){
		i->second->write(data);
	} else {
		fprintf(stderr, "Send to non-existing user.\n");
	}
}

void Hub::broadcast(string data, ADCClient* c)
{
	Buffer::writeBuffer tmp(new Buffer(data, PRIO_NORM));
	if(c == NULL) {
		for(userIter i=users.begin(); i!=users.end(); i++) {
			i->second->writeb(tmp);
		}
	} else {
		for(userIter i=users.begin(); i!=users.end(); i++) {
			if(i->second != c)
				i->second->writeb(tmp);
		}
	}
}

void Hub::broadcastActive(string data, ADCClient* c)
{
	Buffer::writeBuffer tmp(new Buffer(data, PRIO_NORM));
	if(c == NULL) {
		for(userIter i=users.begin(); i!=users.end(); i++) {
			if(i->second->isActive())
				i->second->writeb(tmp);
		}
	} else {
		for(userIter i=users.begin(); i!=users.end(); i++) {
			if(i->second->isActive() && i->second != c)
				i->second->writeb(tmp);
		}
	}
}

void Hub::broadcastPassive(string data, ADCClient* c)
{
	Buffer::writeBuffer tmp(new Buffer(data, PRIO_NORM));
	if(c == NULL) {
		for(userIter i=users.begin(); i!=users.end(); i++){
			if(!i->second->isActive())
				i->second->writeb(tmp);
		}
	} else {
		for(userIter i=users.begin(); i!=users.end(); i++){
			if(!i->second->isActive() && i->second != c)
				i->second->writeb(tmp);
		}
	}
}

void Hub::setMaxPacketSize(int s)
{
	maxPacketSize = s;
}

bool Hub::hasClient(string const& guid) const
{
	if(users.find(guid) == users.end())
		return false;
	return true;
}

void Hub::addClient(string const& guid, ADCClient* client)
{
	assert(!hasClient(guid));
	users[guid] = client;
}

void Hub::removeClient(string const& guid)
{
	assert(hasClient(guid));
	string qui = "IQUI " + guid + " ND\n";
	Buffer::writeBuffer tmp(new Buffer(qui, 0));
	users.erase(guid);
}

ADCClient* Hub::getClient(string const& guid)
{
	userIter i = users.find(guid);
	if(i != users.end())
		return i->second;
	return (ADCClient*)0;
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
