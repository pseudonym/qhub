#ifndef __HUB_H_
#define __HUB_H_

#include "compat_hash_map.h"

#include <list>
#include <string>
#include <boost/shared_ptr.hpp>

#include "Buffer.h"
#include "DNSUser.h"
#include "InterHub.h"
#include "Socket.h"

using namespace std;

namespace qhub {

class ADCClient;

class Hub : public DNSUser {
public:
	static void killAll() throw();

	Hub();
	virtual ~Hub();

	void openADCPort(int port);
	void openInterPort(int port);
	void openInterConnection(string host, int port, string password);
	void onLookup(adns_answer *reply) const;

	string getCID32() const { return "FQI2LLF4K5W3Y"; };

	void setHubName(string name) { Hub::name=name; };
	string getHubName() const { return name; };

	void setMaxPacketSize(int s);
	int getMaxPacketSize() const { return maxPacketSize; };

	void acceptLeaf(int fd, Socket::Domain d);
	void acceptInterHub(int fd, Socket::Domain d);

	bool hasClient(string const& guid) const;
	void addClient(string const& guid, ADCClient* client);
	void removeClient(string const& guid);
	ADCClient* getClient(string const& guid);

	void broadcast(string data, ADCClient* = NULL);
	void broadcastActive(string data, ADCClient* = NULL);
	void broadcastPassive(string data, ADCClient* = NULL);
	void direct(string guid, string data);

	void motd(ADCClient* c);

	void getUsersList(ADCClient* c);
private:
	static void add(Hub* h) throw();
	static void remove(Hub* h) throw();
	typedef list<Hub*> Hubs;
	static Hubs hubs;
	
	string name;

	int maxPacketSize;

	//this is for external users

	//this is for physical users
	hash_map<string, ADCClient*> users;
	typedef hash_map<string, ADCClient*>::iterator userIter;

	//opened connections
	hash_map<string, InterHub*> interConnects;

	//listened opened connections
	list<InterHub*>	interConnects2;
};

}

#endif
