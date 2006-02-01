#include "config.h"
#include "qhub.h"

#include "EventHandler.h"
#include "DNSAdapter.h"
#include "Hub.h"

#include "Plugin.h"
#include "Settings.h"
#include "Logs.h"

using namespace std;
using namespace qhub;

void end(int)
{
	Plugin::deinit();
	Hub::killAll();
	Settings::save();
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
	Settings::parseArgs(argc, argv);

	EventHandler::init();
	DNSAdapter::init();

	Logs::stat << "Starting " PACKAGE_NAME "/" PACKAGE_VERSION << endl;

	Settings::load();

	//try loading
	Plugin::init();
	Plugin::openModule("loader");

	//Init random number generator
	srand(time(NULL));

	EventHandler::mainLoop();

	return EXIT_SUCCESS;
}
