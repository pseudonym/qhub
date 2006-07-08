#include "ClientManager.h"
#include "Util.h"
#include "Encoder.h"
#include "Hub.h"
#include "ADC.h"
#include "UserInfo.h"
#include "ServerManager.h"

using namespace std;
using namespace qhub;

void ClientManager::getUserList(ConnectionBase* c) throw()
{
	Buffer::Ptr t(new Buffer());
	for(LocalUsers::iterator i = localUsers.begin(); i != localUsers.end(); i++) {
		t->append(i->second->getAdcInf());
	}
	for(RemoteUsers::iterator i = remoteUsers.begin(); i != remoteUsers.end(); i++) {
		t->append(i->second->toADC(i->first));
	}
	c->getSocket()->writeb(t);
}

bool ClientManager::hasClient(sid_type sid, bool localonly) const throw()
{
	return localUsers.count(sid) || (localonly ? false : remoteUsers.count(sid));
}

void ClientManager::addLocalClient(sid_type sid, Client* client) throw()
{
	assert(!hasClient(sid));
	localUsers.insert(make_pair(sid, client));
}

void ClientManager::addRemoteClient(sid_type sid, UserInfo const& ui) throw()
{
	assert(!hasClient(sid) || remoteUsers.count(sid));
	if(!remoteUsers.count(sid))
		remoteUsers[sid] = new UserInfo(Command('B', Command::INF, sid));
	remoteUsers[sid]->update(ui);
}

void ClientManager::removeClient(sid_type sid) throw()
{
	assert(hasClient(sid));
	if(localUsers.count(sid)) {
		localUsers.erase(sid);
	} else {
		delete remoteUsers[sid];
		remoteUsers.erase(sid);
	}
}

void ClientManager::getAllInHub(sid_type hsid, std::vector<sid_type>& ret) const throw()
{
	// should not be looking for local users
	ret.clear();
	sid_type mask = ServerManager::instance()->getHubSidMask();
	for(RemoteUsers::const_iterator i = remoteUsers.begin(); i != remoteUsers.end(); ++i)
		if((i->first & mask) == (hsid & mask))
			ret.push_back(i->first);
}

bool ClientManager::hasNick(const string& nick) const throw()
{
	// O(n)... not very pretty
	for(LocalUsers::const_iterator i = localUsers.begin(); i != localUsers.end(); ++i)
		if(nick == i->second->getUserInfo()->getNick())
			return true;
	for(RemoteUsers::const_iterator i = remoteUsers.begin(); i != remoteUsers.end(); ++i)
		if(nick == i->second->getNick())
			return true;
	return false;
}


