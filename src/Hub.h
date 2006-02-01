// vim:ts=4:sw=4:noet
#ifndef __HUB_H_
#define __HUB_H_

#include "compat_hash_map.h"
#include <string>
#include <map>
#include <boost/utility.hpp>

#include "Buffer.h"
#include "UserInfo.h"
#include "Socket.h"
#include "ADCClient.h"

using namespace std;

namespace qhub {

class InterHub;
class ADCSocket;
class ConnectionBase;

class RemoteHub {
public:
	RemoteHub(const string& cid_, const UserInfo& u, InterHub* i) throw()
			: cid(cid_), ui(u), ih(i) {}
	~RemoteHub() throw() {}

	UserInfo* getUserInfo() throw() { return &ui; }
	const string& getCID32() const throw() { return cid; }
	InterHub* getInterHub() throw() { return ih; }

private:
	std::string cid;
	UserInfo ui;
	InterHub* ih;
};

class Hub : boost::noncopyable {
public:
	static void killAll() throw();

	Hub(const string& cid32, const string& name);
	virtual ~Hub();

	void openClientPort(int port);
	void openInterPort(int port);
	void openInterConnection(const string& host, int port, const string& pass) throw();

	//these should never change during the life of the hub
	const string& getCID32() const { return cid32; };
	const string& getHubName() const { return name; };

	void setDescription(const string& d) { description = d; };
	const string& getDescription() const { return description; };

	void setInterPass(const string& p) { interPass = p; }
	const string& getInterPass() const { return interPass; }

	void acceptLeaf(int fd, Socket::Domain d);
	void acceptInterHub(int fd, Socket::Domain d);

	void activate(InterHub* ih) throw();
	void deactivate(InterHub* ih) throw();

	bool hasClient(string const& cid, bool localonly = false) const throw();
	void addActiveClient(string const& cid, ADCClient* client) throw();
	void addPassiveClient(string const& cid, ADCClient* client) throw();
	void addRemoteClient(string const& cid, UserInfo const&) throw();
	void removeClient(string const& cid) throw();
	void switchClientMode(bool toActive, string const& cid, ADCClient* client) throw();

	void addRemoteHub(string const& cid, UserInfo const& ui, InterHub* conn) throw();
	void removeRemoteHub(string const& cid) throw();

	void broadcast(string const& data, ConnectionBase* except = NULL) throw();
	void broadcastActive(string const& data, ConnectionBase* except = NULL) throw();
	void broadcastPassive(string const& data, ConnectionBase* except = NULL) throw();
	void broadcastInter(string const& data, InterHub* except = NULL) throw();
	void broadcastFeature(string const& data, string const& feat, bool yes, ConnectionBase* except = NULL) throw();
	void direct(string const& data, string const& cid, Client* from = NULL) throw();

	void motd(ADCClient* c) throw();

	bool hasNick(const string& nick) throw();

	void getUserList(ConnectionBase* c) throw();
	void getInterList(InterHub* ih) throw();

	void userDisconnect(string const& actor, string const& victim, string const& msg) throw();
private:
	static void add(Hub* h) throw();
	static void remove(Hub* h) throw();

	typedef vector<Hub*> Hubs;
	static Hubs hubs;

	const string name;
	const string cid32;
	string description;
	string interPass;
	
	typedef hash_map<string, ADCClient*> Users;
	Users passiveUsers;
	Users activeUsers;

	typedef hash_map<string, UserInfo> RemoteUsers;
	RemoteUsers remoteUsers;

	// for hubs on the network
	typedef map<string, RemoteHub*> RemoteHubs;
	RemoteHubs remoteHubs;

	//for actual connections
	typedef vector<InterHub*> Interhubs;
	//working connections
	Interhubs interhubs;
};

}

#endif
