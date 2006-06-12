#ifndef QHUB_CLIENTMANAGER_H
#define QHUB_CLIENTMANAGER_H

#include "compat_hash_map.h"
#include "Singleton.h"
#include "id.h"
#include "Client.h"

namespace qhub {

class ClientManager : public Singleton<ClientManager> {
public:
	bool hasClient(sid_type sid, bool localonly = false) const throw();
	void addLocalClient(sid_type sid, Client* client) throw();
	void addRemoteClient(sid_type sid, UserInfo const&) throw();
	void removeClient(sid_type sid) throw();
	void getAllInHub(sid_type, std::vector<sid_type>&) const throw();

	void getUserList(ConnectionBase*) throw();

	bool hasNick(const std::string& nick) const throw();

	typedef std::hash_map<sid_type, Client*> LocalUsers;
	typedef std::hash_map<sid_type, UserInfo*> RemoteUsers;
private:
	friend void dispatch(const Command&, ConnectionBase*) throw();
	friend class Singleton<ClientManager>;

	LocalUsers localUsers;

	RemoteUsers remoteUsers;
};

} // namespace qhub

#endif // QHUB_CLIENTMANAGER_H
