#ifndef __HUB_H_
#define __HUB_H_

#include "compat_hash_map.h"

#include <list>
#include <string>
#include <boost/shared_ptr.hpp>

#include "Buffer.h"

using namespace std;

namespace qhub {

class ADC;

class Hub {
public:
	Hub();

	void acceptLeaf(int fd);
	void acceptInterHub(int fd);

	bool addClient(ADC* client, string guid);
	
	void broadcast(ADC* c, string data);
	void broadcastSelf(ADC* c, string data);
	void direct(string guid, string data);

	void getUsersList(ADC* c);
private:
	//this is for external users
		
	//this is for physical users
	hash_map<string, ADC*> users;
	typedef hash_map<string, ADC*>::iterator userIter;

	//userlist-cache. this is the ones thats OUT at the clients´
	qhub::Buffer::writeBuffer userlist;
	list<qhub::Buffer::writeBuffer> outliers;

	void createCache();
};

}

#endif
