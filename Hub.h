#ifndef __HUB_H_
#define __HUB_H_

#include "compat_hash_map.h"

#include <string>

using namespace std;

namespace qhub {

class ADC;

class Hub {
public:
	Hub();

	void acceptLeaf(int fd);
	void acceptInterHub(int fd);

	void addClient(ADC* client, string nick);

private:
	hash_map<string, ADC*>	users;
};

}

#endif
