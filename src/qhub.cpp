#include "config.h"

#include "EventHandler.h"
#include "DNSAdapter.h"
#include "Hub.h"

#include "Plugin.h"
#include "ClientManager.h"
#include "ServerManager.h"
#include "ConnectionManager.h"
#include "Settings.h"
#include "Logs.h"
#include "Command.h"
#include "InterHub.h"

using namespace std;
using namespace qhub;

void end(int)
{
	Plugin::deinit();
	//Hub::killAll();
	Settings::instance()->save();
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	signal(SIGINT, &end);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
#ifdef LINUX
	signal(SIGCLD, SIG_IGN);
#else
	signal(SIGCHLD, SIG_IGN);
#endif

	//do this here so we don't wind up doing extra
	//work if all they want is --version or --help
	Settings::instance()->parseArgs(argc, argv);

	EventHandler::init();
	DNSAdapter::init();

	Logs::stat << "Starting " PACKAGE_NAME "/" PACKAGE_VERSION << endl;

	//Settings::load(); // will already be loaded by constructor
	Hub::instance();
	ClientManager::instance();
	ConnectionManager::instance();
	ServerManager::instance();

	//try loading
	Plugin::init();
	Plugin::openModule("loader");

	//Init random number generator
	srand(time(NULL));

	EventHandler::mainLoop();

	return EXIT_SUCCESS;
}

namespace qhub {
#if 1
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
		if(us.count(cmd.getDest())) {
			if(us.count(cmd.getSource()))
				us.find(cmd.getSource())->second->getSocket()->writeb(tmp);
			us.find(cmd.getDest())->second->getSocket()->writeb(tmp);
		} else if(others.count(cmd.getDest())) {
			sid_type s = cmd.getDest();
			// set last two bytes to "AA"
			*(reinterpret_cast<uint16_t*>(&s)+1) = 0x4141;
			if(us.count(cmd.getSource()))
				us.find(cmd.getSource())->second->getSocket()->writeb(tmp);
			ServerManager::instance()->remoteHubs[s]->getInterHub()->getSocket()->writeb(tmp);
		} else {
			assert(0);
		}
		break;
	case 'F':
		for(Interhubs::const_iterator i = ih.begin(); i != ih.end(); ++i)
			if(*i != except)
				(*i)->getSocket()->writeb(tmp);
		for(LocalUsers::const_iterator i = us.begin(); i != us.end(); ++i) {
			const string& feat = cmd.getFeatures();
		}
		break;
	}
}
#endif
} // namespace qhub
