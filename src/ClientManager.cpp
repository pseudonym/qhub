// vim:ts=4:sw=4:noet
#include "ClientManager.h"

#include "ADC.h"
#include "Client.h"
#include "ConnectionBase.h"
#include "Logs.h"
#include "ServerManager.h"
#include "UserInfo.h"
#include "Util.h"
#include "ZBuffer.h"

using namespace std;
using namespace qhub;

void ClientManager::getUserList(ConnectionBase* c) throw()
{
	// if we can, compress the user list
	// TODO cache this, and rebuild every once in a while
	// like Aquila does
	if(c->hasSupport("ZLIF")) {
		ZBuffer::MutablePtr t(new ZBuffer);
		fillUserListBuf(t);
		t->finalize();
		c->getSocket()->writeb(t);
	} else {
		Buffer::MutablePtr t(new Buffer);
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

void ClientManager::userUpdated(sid_type sid, UserInfo const& ui) throw()
{
	LocalUsers::iterator i = localUsers.find(sid);
	if(i == localUsers.end())
		assert(0); // maybe use this for remote later, but now...

	const string& oldNick = i->second->getUserInfo()->getNick();
	const string& newNick = ui.getNick();
	if(ui.has("NI")) {
		nicks.erase(oldNick);
		nicks.insert(newNick);
	}
	// CID can't change, nothing else to do
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

void ClientManager::broadcast(const Command& cmd) throw()
{
	// should be safe to delay these
	if(cmd.getCmd() == Command::SCH || cmd.getCmd() == Command::INF) {
		if(broadcastQueue.empty())
			EventManager::instance()->addTimer(this, 0, 5); // FIXME allow timeout to be settable
		broadcastQueue.push_back(cmd);
	} else {
		broadcastQueue.push_back(cmd);
		EventManager::instance()->removeTimer(this);
		purgeQueue();
	}
}

void ClientManager::onTimer(int) throw()
{
	purgeQueue();
}

void ClientManager::purgeQueue() throw()
{
	typedef vector<Command>::const_iterator QI;

	Buffer::MutablePtr tmp(new Buffer);
	ZBuffer::MutablePtr ztmp;
	for(QI i = broadcastQueue.begin(); i != broadcastQueue.end(); ++i)
		tmp->append(*i);

	bool use_z = tmp->size() > 1024; // FIXME user-settable

	if(use_z) {
		ztmp.reset(new ZBuffer);
		for(QI i = broadcastQueue.begin(); i != broadcastQueue.end(); ++i)
			ztmp->append(*i);
		ztmp->finalize();
	}

	typedef LocalUsers::const_iterator CI;
	for(CI i = localUsers.begin(); i != localUsers.end(); ++i)
		if(use_z && i->second->hasSupport("ZLIF"))
			i->second->getSocket()->writeb(ztmp);
		else
			i->second->getSocket()->writeb(tmp);

	broadcastQueue.clear();
}

void ClientManager::broadcastFeature(const Command& cmd) throw()
{
	Buffer::Ptr tmp(new Buffer(cmd));
	const string& feat = cmd.getFeatures();
	typedef LocalUsers::const_iterator CI;

	for(CI i = localUsers.begin(); i != localUsers.end(); ++i) {
		Client* c = i->second;
		bool send = true;
		for(string::const_iterator j = feat.begin(); j != feat.end(); j += 5) {
			bool noSupp = *j == '+' && !c->getUserInfo()->hasSupport(string(j+1, j+5));
			bool negSupp = *j == '-' && c->getUserInfo()->hasSupport(string(j+1, j+5));
			if (noSupp || negSupp) {
				Logs::line << "not sending to user " << ADC::fromSid(c->getSid())
						<< " because of feature " << string(j+1, j+5) << endl;
				send = false;
				break;
			}
		}
		if(send)
			c->getSocket()->writeb(tmp);
	}
}

void ClientManager::direct(const Command& cmd) throw()
{
	LocalUsers::const_iterator i = localUsers.find(cmd.getDest());
	if(i != localUsers.end()) {
		i->second->send(cmd);
		return;
	}
	RemoteUsers::const_iterator j = remoteUsers.find(cmd.getDest());
	if(j != remoteUsers.end()) {
		sid_type s = cmd.getDest();
		s &= ServerManager::instance()->getHubSidMask();
		ServerManager::instance()->direct(s, cmd);
	}
}

