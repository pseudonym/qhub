#ifndef __HUB_H_
#define __HUB_H_

#include "compat_hash_map.h"

#include <string>
#include <boost/shared_ptr.hpp>

using namespace std;

namespace qhub {

class ADC;

class Hub {
public:
	typedef boost::shared_ptr<string> Buffer;

	Hub();

	void acceptLeaf(int fd);
	void acceptInterHub(int fd);

	bool addClient(ADC* client, string guid);
	

	void broadcast(ADC* c, string data);
	void broadcastSelf(ADC* c, string data);
	Buffer getUsersList();

	void direct(string guid, string data);

		
private:
	hash_map<string, ADC*> users;
	typedef hash_map<string, ADC*>::iterator userIter;
};

}

#endif
