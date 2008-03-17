#include "config.h"

#include "EventManager.h"
#include "Hub.h"

#include "Plugin.h"
#include "ClientManager.h"
#include "ServerManager.h"
#include "ConnectionManager.h"
#include "PluginManager.h"
#include "Settings.h"
#include "Logs.h"
#include "Command.h"
#include "InterHub.h"

using namespace std;
using namespace qhub;

class SigHandler : public EventListener {
public:
	void onSignal(int) throw();
};

void SigHandler::onSignal(int sig) throw()
{
	switch(sig) {
	case SIGINT:
	case SIGTERM:
		PluginManager::instance()->removeAll();
		Settings::instance()->save();
		exit(EXIT_SUCCESS);
	case SIGPIPE:
	case SIGALRM:
	case SIGCHLD:
		//do nothing
		break;
	default:
		assert(0 && "unknown signal received");
	}
}

int main(int argc, char **argv)
{
	Logs::stat << "Starting " PACKAGE_NAME "/" PACKAGE_VERSION << endl;

	// do this here so we don't wind up doing extra
	// work if all they want is --version or --help
	Settings::instance()->parseArgs(argc, argv);


	// make sure these are actually instantiated; their constructors
	// load all of the configuration and bootstrap everything
	Hub::instance();
	ClientManager::instance();
	ConnectionManager::instance();
	ServerManager::instance();

	// try loading
	PluginManager::instance()->open("loader");

	// Init random number generator
	srand(time(NULL));

	// kind of ugly, should switch to setting these in
	// Manager classes somewhere
	SigHandler sh;
	EventManager::instance()->addSignal(SIGINT, &sh);
	EventManager::instance()->addSignal(SIGTERM, &sh);
	EventManager::instance()->addSignal(SIGPIPE, &sh);
	EventManager::instance()->addSignal(SIGALRM, &sh);
	EventManager::instance()->addSignal(SIGCHLD, &sh);

	return EventManager::instance()->run();
}

#if 0
namespace qhub {

void dispatch(Command const& cmd, ConnectionBase* except /*=NULL*/) throw()
{
	typedef ServerManager::Interhubs Interhubs;
	typedef ClientManager::LocalUsers LocalUsers;
	typedef ClientManager::RemoteUsers RemoteUsers;
	const Interhubs& ih = ServerManager::instance()->interhubs;
	const LocalUsers& us = ClientManager::instance()->localUsers;
	const RemoteUsers& others = ClientManager::instance()->remoteUsers;
	Buffer::Ptr tmp(new Buffer(cmd));
	switch(cmd.getAction()) {
	case 'I':
	case 'B':
		for(LocalUsers::const_iterator i = us.begin(); i != us.end(); ++i)
			if(i->second != except)
				i->second->getSocket()->writeb(tmp);
	case 'S':
		for(Interhubs::const_iterator i = ih.begin(); i != ih.end(); ++i)
			if(*i != except)
				(*i)->getSocket()->writeb(tmp);
		break;
	case 'D':
	case 'E':
		if(us.count(cmd.getDest())) {
			if(us.count(cmd.getSource()) && cmd.getAction() == 'E')
				us.find(cmd.getSource())->second->getSocket()->writeb(tmp);
			us.find(cmd.getDest())->second->getSocket()->writeb(tmp);
		} else if(others.count(cmd.getDest())) {
			sid_type s = cmd.getDest();
			s &= ServerManager::instance()->getHubSidMask();
			if(us.count(cmd.getSource()) && cmd.getAction() == 'E')
				us.find(cmd.getSource())->second->getSocket()->writeb(tmp);
			ServerManager::instance()->remoteHubs[s]->getInterHub()->getSocket()->writeb(tmp);
		} else {
			return;
		}
		break;
	case 'F':
		for(Interhubs::const_iterator i = ih.begin(); i != ih.end(); ++i)
			if(*i != except)
				(*i)->getSocket()->writeb(tmp);
		const string& feat = cmd.getFeatures();
		for(LocalUsers::const_iterator i = us.begin(); i != us.end(); ++i) {
			Client* c = i->second;
			bool send = true;
			for(string::size_type j = 0; j < feat.size(); j += 5) {
				if(feat[j] == '+' && c->getUserInfo()->hasSupport(feat.substr(j+1, 4)))
					send = true;
				else if(feat[j] == '-' && !c->getUserInfo()->hasSupport(feat.substr(j+1, 4)))
					send = true;
				else {
					Logs::line << "not sending to user " << ADC::fromSid(c->getSid())
							<< "because of feature " << feat.substr(j+1, 4) << endl;
					send = false;
					break;
				}
			}
			if(send)
				c->getSocket()->writeb(tmp);
		}
		break;
	}
}

} // namespace qhub
#endif
