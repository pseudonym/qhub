// vim:ts=4:sw=4:noet
#include "qhub.h"
#include "ClientManager.h"
#include "ConnectionManager.h"
#include "EventManager.h"
#include "Hub.h"
#include "Logs.h"
#include "PluginManager.h"
#include "ServerManager.h"
#include "Settings.h"

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
