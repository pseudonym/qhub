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

Hub::Buffer Hub::getUsersList()
{
//Good algorithm for this?
//checkpointing + DIFFs could work. (Why? Limits the amount of (re-) iterating over nicks.)
//caching this way reduces iteration over a random timeinterval from n^2 to n, where n is number of users
//drawbacks include more small sends to do (can be easily merged though, remembering last size of "quits+joins"
//part
//methods to limit the amount of joins to begin with could help efficiency
	string *tmp = new string();
	for(userIter i=users.begin(); i!=users.end(); i++){
		tmp->append(i->second->getFullInf());
	}

	return Buffer(tmp);
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
	fprintf(stderr, "Broadcasting 1 %d\n", users.size());
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
	return true;
}

void Hub::acceptInterHub(int fd)
{
	Socket* tmp = new InterHub(fd);
}


