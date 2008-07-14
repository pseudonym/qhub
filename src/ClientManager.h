#ifndef QHUB_CLIENTMANAGER_H
#define QHUB_CLIENTMANAGER_H

#include "qhub.h"
#include "fast_map.h"
#include "fast_set.h"
#include "Buffer.h"
#include "Command.h"
#include "EventManager.h"
#include "Singleton.h"

#include <string>
#include <vector>

namespace qhub {

class ClientManager : public Singleton<ClientManager>, public EventListener {
public:
	bool hasClient(sid_type sid, bool localonly = false) const throw();
	void addLocalClient(sid_type sid, Client* client) throw();
	void userUpdated(sid_type sid, UserInfo const&) throw();
	void addRemoteClient(sid_type sid, UserInfo const&) throw();
	void removeClient(sid_type sid) throw();
	void getAllInHub(sid_type, std::vector<sid_type>&) const throw();

	void getUserList(ConnectionBase*) throw();

	void broadcast(const Command&) throw();
	virtual void onTimer(int) throw();
	void purgeQueue() throw();

	void broadcastFeature(const Command&) throw();
	void direct(const Command&) throw();

	bool hasNick(const std::string& nick) const throw() { return nicks.count(nick); }
	bool hasCid(const std::string& cid) const throw() { return cids.count(cid); }

	typedef fast_map<sid_type, Client*>::type LocalUsers;
	typedef fast_map<sid_type, UserInfo*>::type RemoteUsers;
private:
	friend class Singleton<ClientManager>;

	void fillUserListBuf(Buffer::MutablePtr);

	LocalUsers localUsers;

	RemoteUsers remoteUsers;

	fast_set<std::string>::type nicks;
	fast_set<std::string>::type cids;

	std::vector<Command> broadcastQueue;

	ClientManager() throw() {}
	~ClientManager() throw() {}
};

} // namespace qhub

#endif // QHUB_CLIENTMANAGER_H
