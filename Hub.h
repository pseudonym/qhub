// vim:ts=4:sw=4:noet
#ifndef __HUB_H_
#define __HUB_H_

#include "compat_hash_map.h"

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>

#include "Buffer.h"
#include "DNSUser.h"
#include "InterHub.h"
#include "UserInfo.h"
#include "Socket.h"
#include "ADCSocket.h"
#include "ADCClient.h"

using namespace std;

namespace qhub {

class Hub {
public:
	static void killAll() throw();

	Hub();
	virtual ~Hub();

	void openADCPort(int port);
	void openInterPort(int port);
	void openInterConnection(const string& host, int port) throw();

	void setCID32(const string& c) { cid32 = c; };
	const string& getCID32() const { return cid32; };

	void setHubName(const string& n) { name = n; };
	const string& getHubName() const { return name; };

	void setDescription(const string& d) { description = d; };
	const string& getDescription() const { return description; };

	void setMaxPacketSize(int s);
	int getMaxPacketSize() const { return maxPacketSize; };

	void acceptLeaf(int fd, Socket::Domain d);
	void acceptInterHub(int fd, Socket::Domain d);

	void activate(InterHub* ih) throw();
	void deactivate(InterHub* ih) throw();

	bool hasClient(string const& cid, bool localonly = false) const throw();
	void addActiveClient(string const& cid, ADCClient* client) throw();
	void addPassiveClient(string const& cid, ADCClient* client) throw();
	void removeClient(string const& cid) throw();
	void switchClientMode(bool toActive, string const& cid, ADCClient* client) throw();

	void broadcast(string const& data, ADCClient* except = NULL, bool localonly = false) throw();
	void broadcastActive(string const& data, bool localonly = false) throw();
	void broadcastPassive(string const& data, bool localonly = false) throw();
	void direct(string const& data, string const& cid, ADCClient* from = NULL) throw();

	void motd(ADCClient* c) throw();

	void getUserList(ADCSocket* c, bool localonly = false) throw();

	void userDisconnect(string const& actor, string const& victim, string const& msg) throw();
private:
	static void add(Hub* h) throw();
	static void remove(Hub* h) throw();
	inline size_t motdHelper();

	typedef list<Hub*> Hubs;
	static Hubs hubs;

	int maxPacketSize;
	
	string name;
	string cid32;
	string description;
	
	typedef hash_map<string, ADCClient*> Users;
	//this is for external users
	//Users remoteUsers;

	//this is for physical users
	Users passiveUsers;
	Users activeUsers;

	typedef list<InterHub*> Interhubs;
	//working connections
	Interhubs interhubs;
	//waiting for handshake
	//Interhubs pendingInterhubs;
	//it will add itself when it's ready to handle data, otherwise, it's on its own
};

}

#endif
