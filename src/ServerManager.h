#ifndef QHUB_SERVERMANAGER_H
#define QHUB_SERVERMANAGER_H

#include "Singleton.h"
#include "id.h"
#include "UserInfo.h"
#include <string>
#include <map>
#include <vector>

namespace qhub {

class InterHub;
class Command;
class ConnectionBase;

extern void dispatch(const Command&, ConnectionBase*) throw();

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
	void direct(const Command&, const string&) throw();
	void getInterList(InterHub* ih) throw();
	void activate(InterHub* ih) throw();
	void deactivate(InterHub* ih) throw();
	bool hasServer(sid_type) const throw();

	typedef map<sid_type, RemoteHub*> RemoteHubs;
	typedef vector<InterHub*> Interhubs;
private:
	friend void dispatch(const Command&, ConnectionBase*) throw();
	friend class Singleton<ServerManager>;

	// for hubs on the network
	RemoteHubs remoteHubs;

	//for actual connections
	Interhubs interhubs;
};

} // namespace qhub

#endif // QHUB_SERVERMANAGER_H
