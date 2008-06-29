#ifndef QHUB_SERVERMANAGER_H
#define QHUB_SERVERMANAGER_H

#include "qhub.h"
#include "Singleton.h"
#include "UserInfo.h"

#include <map>
#include <string>
#include <vector>

namespace qhub {

class RemoteHub {
public:
	RemoteHub(const UserInfo& u, InterHub* i) throw()
			: ui(u), ih(i) {}
	~RemoteHub() throw() {}

	UserInfo* getUserInfo() throw() { return &ui; }
	InterHub* getInterHub() throw() { return ih; }

private:
	UserInfo ui;
	InterHub* ih;
};

class ServerManager : public Singleton<ServerManager> {
public:
	void add(sid_type sid, UserInfo const& ui, InterHub* conn) throw();
	void remove(sid_type sid) throw();
	void getInterList(InterHub* ih) throw();
	void activate(InterHub* ih) throw();
	void deactivate(InterHub* ih) throw();
	bool hasServer(sid_type) const throw();
	sid_type getHubSidMask() const throw() { return sidMask; }
	sid_type getClientSidMask() const throw() { return ~getHubSidMask(); }

	void broadcast(const Command&, ConnectionBase* except) throw();
	void direct(sid_type s, const Command&) throw();

	typedef std::map<sid_type, RemoteHub*> RemoteHubs;
	typedef std::vector<InterHub*> Interhubs;
private:
	friend class Singleton<ServerManager>;

	sid_type sidMask;

	// for hubs on the network
	RemoteHubs remoteHubs;

	//for actual connections
	Interhubs interhubs;

	ServerManager() throw();
	~ServerManager() throw() {}
};

} // namespace qhub

#endif // QHUB_SERVERMANAGER_H
