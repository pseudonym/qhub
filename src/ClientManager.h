#ifndef QHUB_CLIENTMANAGER_H
#define QHUB_CLIENTMANAGER_H

#include "compat_hashtable.h"
#include "Singleton.h"
#include "id.h"
#include "Client.h"
#include "Buffer.h"

namespace qhub {

class ClientManager : public Singleton<ClientManager> {
public:
	bool hasClient(sid_type sid, bool localonly = false) const throw();
	void addLocalClient(sid_type sid, Client* client) throw();
	void addRemoteClient(sid_type sid, UserInfo const&) throw();
	void removeClient(sid_type sid) throw();
	void getAllInHub(sid_type, std::vector<sid_type>&) const throw();

	void getUserList(ConnectionBase*) throw();

	void broadcast(const Command&) throw();
	void broadcastFeature(const Command&) throw();
	void direct(const Command&) throw();

	bool hasNick(const std::string& nick) const throw() { return nicks.count(nick); }
	bool hasCid(const cid_type& cid) const throw() { return cids.count(cid); }

	typedef std::hash_map<sid_type, Client*> LocalUsers;
	typedef std::hash_map<sid_type, UserInfo*> RemoteUsers;
private:
	friend class Singleton<ClientManager>;

	void fillUserListBuf(Buffer::MutablePtr);

	LocalUsers localUsers;

	RemoteUsers remoteUsers;

    std::hash_set<string> nicks;
    std::hash_set<cid_type> cids;
};

} // namespace qhub

#endif // QHUB_CLIENTMANAGER_H
