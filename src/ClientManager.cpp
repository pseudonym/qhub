#include "ClientManager.h"
#include "Util.h"
#include "Encoder.h"
#include "Hub.h"
#include "ADC.h"
#include "UserInfo.h"
#include "ServerManager.h"
#include "ZBuffer.h"

using namespace std;
using namespace qhub;

void ClientManager::getUserList(ConnectionBase* c) throw()
{
	// if we can, compress the user list
	// TODO cache this, and rebuild every once in a while
	// like Aquila does
	if(c->hasSupport("ZLIF")) {
		boost::shared_ptr<ZBuffer> t(new ZBuffer());
		fillUserListBuf(t);
		t->finalize();
		c->getSocket()->writeb(t);
	} else {
		Buffer::MutablePtr t(new Buffer());
		fillUserListBuf(t);
		c->getSocket()->writeb(t);
	}
}

void ClientManager::fillUserListBuf(Buffer::MutablePtr t)
{
	for(LocalUsers::iterator i = localUsers.begin(); i != localUsers.end(); i++) {
		t->append(i->second->getAdcInf());
	}
	for(RemoteUsers::iterator i = remoteUsers.begin(); i != remoteUsers.end(); i++) {
		t->append(i->second->toADC(i->first));
	}
}

bool ClientManager::hasClient(sid_type sid, bool localonly) const throw()
{
	return localUsers.count(sid) || (localonly ? false : remoteUsers.count(sid));
}

void ClientManager::addLocalClient(sid_type sid, Client* client) throw()
{
	assert(!hasClient(sid));
	localUsers.insert(make_pair(sid, client));
    nicks.insert(client->getUserInfo()->getNick());
    cids.insert(client->getUserInfo()->getCID());
}

void ClientManager::addRemoteClient(sid_type sid, UserInfo const& ui) throw()
{
	assert(!hasClient(sid) || remoteUsers.count(sid));
	if(!remoteUsers.count(sid))
		remoteUsers[sid] = new UserInfo(Command('B', Command::INF, sid));
    if(ui.has("ID") && remoteUsers[sid]->has("ID")) {
        cids.erase(remoteUsers[sid]->getCID());
        cids.insert(ui.getCID());
    }
    if(ui.has("NI") && remoteUsers[sid]->has("NI")) {
        nicks.erase(remoteUsers[sid]->getNick());
        nicks.insert(ui.getNick());
    }
	remoteUsers[sid]->update(ui);
}

void ClientManager::removeClient(sid_type sid) throw()
{
	assert(hasClient(sid));
	if(localUsers.count(sid)) {
        UserInfo* i = localUsers[sid]->getUserInfo();
        nicks.erase(i->getNick());
        cids.erase(i->getCID());
		localUsers.erase(sid);
	} else {
        UserInfo* i = remoteUsers[sid];
        nicks.erase(i->getNick());
        cids.erase(i->getCID());
		delete i;
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

