#include "Hub.h"

#include "qhub.h"
#include "ServerSocket.h"
#include "ADC.h"
#include "InterHub.h"

using namespace qhub;

Hub::Hub() 
{
	//Inter-hub
	ServerSocket* tmp = new ServerSocket(9000, ServerSocket::INTER_HUB, this);
	enable(tmp->getFd(), OOP_READ, tmp);

	//Leaf-handler
	tmp = new ServerSocket(9001, ServerSocket::LEAF_HANDLER, this);
	enable(tmp->getFd(), OOP_READ, tmp);
}

void Hub::acceptLeaf(int fd)
{
	Socket* tmp = new ADC(fd, this);
}

void Hub::createCache()
{
	//rebuild master cache
	string tmp;
	for(userIter i=users.begin(); i!=users.end(); i++){
		tmp += i->second->getFullInf();
	}
	
	userlist.reset(new Buffer(tmp, 0));

	outliers.clear();

	fprintf(stderr, "Created a cache: %s\n", tmp.c_str());
}

void Hub::getUsersList(ADC* c)
{
//Good algorithm for this?
//checkpointing + DIFFs could work. (Why? Limits the amount of (re-) iterating over nicks.)
//caching this way reduces iteration over a random timeinterval from n^2 to n, where n is number of users
//drawbacks include more small sends to do (can be easily merged though, remembering last size of "quits+joins"
//part
//methods to limit the amount of joins to begin with could help efficiency
	if(userlist.get() == NULL /*|| oldcache*/){
		createCache();
	}

	//send userlist
	c->w(userlist);
	for(list<qhub::Buffer::writeBuffer>::iterator i=outliers.begin(); i!=outliers.end(); i++){
		c->w(*i);
	}
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

void Hub::broadcast(ADC* c, string data)
{
	Buffer::writeBuffer tmp(new Buffer(data, 0));
	fprintf(stderr, "Broadcasting to %d users.\n", users.size());
	for(userIter i=users.begin(); i!=users.end(); i++){
		if(i->second != c){
			i->second->write(data);
		}
	}
}

void Hub::broadcastSelf(ADC* c, string data)
{
	for(userIter i=users.begin(); i!=users.end(); i++){
		i->second->write(data);
	}
}

bool Hub::addClient(ADC* client, string guid)
{
	if(users.find(guid) != users.end()){
		fprintf(stderr, "GUID collision.\n");
		return false;
	}
	users[guid] = client;

	//add to userlist cache, dont check age here (?)
	if(userlist.get() == NULL){
		createCache();
	} else {
		Buffer::writeBuffer tmp(new Buffer(client->getFullInf(), 0));
		outliers.push_back(tmp);
	}

	return true;
}

void Hub::acceptInterHub(int fd)
{
	Socket* tmp = new InterHub(fd);
}


