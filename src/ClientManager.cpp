#include "ClientManager.h"
#include "Util.h"
#include "Encoder.h"
#include "Hub.h"
#include "ADC.h"
#include "UserInfo.h"
#include "ServerManager.h"

using namespace std;
using namespace qhub;

sid_type ClientManager::nextSid() throw()
{
	sid_type sid;
	const string& pre = Hub::instance()->getSidPrefix();
	while(true) {
		vector<uint8_t> r = Util::genRand(2);
		const string& post = Encoder::toBase32(&r[0], r.size()).substr(0, 2);
		sid = ADC::toSid(pre + post);
		if(hasClient(sid) || post == "AA")
			continue;
		return sid;
	}
}

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

void ClientManager::direct(Command const& data, sid_type sid, Client* from /*=NULL*/) throw()
{
	dispatch(data, from);
/*	if(localUsers.count(sid)) {
		Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
		if(from)
			from->getSocket()->writeb(tmp);
		localUsers[sid]->getSocket()->writeb(tmp);
	} else if(remoteUsers.count(sid)) {
		ServerManager::instance()->direct(data, remoteUsers[sid]->getRealHub());
	}*/
}

void ClientManager::broadcast(Command const& data, Client* except/* = NULL*/) throw()
{
	dispatch(data, except);
/*	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	for(LocalUsers::iterator i = localUsers.begin(); i != localUsers.end(); ++i) {
		if(i->second != except)
			i->second->getSocket()->writeb(tmp);
	}*/
}

void ClientManager::broadcastFeature(Command const& data, Client* except/* = NULL*/) throw()
{
	broadcast(data, except);
/*	Buffer::Ptr tmp(new Buffer(data, PRIO_NORM));
	for(LocalUsers::iterator i = localUsers.begin(); i != localUsers.end(); ++i) {
		if(i->second->getUserInfo()->hasSupport(feat) == yes)
			i->second->getSocket()->writeb(tmp);
	}*/
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

void ClientManager::getAllInHub(const std::string& pre, std::vector<sid_type>& ret) const throw()
{
	// should not be looking for local users
	ret.clear();
	for(RemoteUsers::const_iterator i = remoteUsers.begin(); i != remoteUsers.end(); ++i)
		if(i->second->getRealHub() == pre)
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


