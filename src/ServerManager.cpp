#include "ServerManager.h"
#include "InterHub.h"
#include "XmlTok.h"
#include "Settings.h"

using namespace std;
using namespace qhub;

ServerManager::ServerManager() throw() : sidMask(0xFFFFFFFF)
{
	XmlTok* p = Settings::instance()->getConfig("__hub");
	int i = Util::toInt(p->getAttr("hubsidbits"));
	sidMask >>= 20 - i;
	sidMask <<= 20 - i;
}

void ServerManager::getInterList(InterHub* ih) throw()
{
	for(RemoteHubs::iterator i = remoteHubs.begin(); i != remoteHubs.end(); ++i) {
		ih->send(i->second->getUserInfo()->toADC(i->first));
	}
}

void ServerManager::add(sid_type sid, const UserInfo& ui, InterHub* conn) throw()
{
	if(remoteHubs.count(sid)) {
		assert(conn == remoteHubs[sid]->getInterHub());
		remoteHubs[sid]->getUserInfo()->update(ui);
	} else {
		remoteHubs.insert(make_pair(sid, new RemoteHub(ui, conn)));
	}
}

void ServerManager::remove(sid_type sid) throw()
{
	RemoteHubs::iterator i = remoteHubs.find(sid);
	if(i != remoteHubs.end()) {
		delete i->second;
		remoteHubs.erase(i);
	}
}

void ServerManager::activate(InterHub* ih) throw()
{
	interhubs.push_back(ih);
}

void ServerManager::deactivate(InterHub* ih) throw()
{
	Interhubs::iterator i = find(interhubs.begin(), interhubs.end(), ih);
	if(i != interhubs.end())
		interhubs.erase(i);
}

bool ServerManager::hasServer(sid_type sid) const throw()
{
	return remoteHubs.count(sid);
}
