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

	bool hasClient(string const& guid) const throw();
	void addActiveClient(string const& guid, ADCClient* client) throw();
	void addPassiveClient(string const& guid, ADCClient* client) throw();
	void removeClient(string const& guid) throw();
	void switchClientMode(bool toActive, string const& guid, ADCClient* client) throw();
	ADCClient* getClient(string const& guid) throw();

	void broadcast(string const& data, ADCClient* except = NULL) throw();
	void broadcastActive(string const& data) throw();
	void broadcastPassive(string const& data) throw();
	void direct(string const& data, string const& guid, ADCClient* from) throw();

	void motd(ADCClient* c) throw();

	void getUsersList(ADCClient* c) throw();
private:
	static void add(Hub* h) throw();
	static void remove(Hub* h) throw();
	typedef list<Hub*> Hubs;
	static Hubs hubs;
	
	string name;

	int maxPacketSize;

	//this is for external users

	//this is for physical users
	typedef hash_map<string, ADCClient*> Users;
	Users passiveUsers;
	Users activeUsers;

	//opened connections
	hash_map<string, InterHub*> interConnects;

	//listened opened connections
	list<InterHub*>	interConnects2;
};

}

#endif
