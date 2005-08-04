#include "config.h"
#include "qhub.h"
#include "error.h"

#include <stdio.h>

#include "EventHandler.h"
#include "DNSUser.h"
#include "Hub.h"
#include "Timer.h"

#include "Plugin.h"
#include "Settings.h"
#include "Util.h"
#include "Logs.h"

#include <string>

using namespace std;

extern "C" {
#include <oop.h>
#include <oop-read.h>

#include <oop-adns.h>
#ifdef HAVE_LIBOOP_EVENT 
#include <oop-event.h>
#endif
}

#ifdef HAVE_LIBOOP_EVENT
#include <event.h>
#endif

using namespace qhub;

struct timer {
	struct timeval tv;
	int delay;
};

void qhub::lookup(const char* hostname, DNSUser* const d){
#ifdef LIBOOP_RECENT
//	oop_adns_submit(adns,NULL,hostname,adns_r_a,adns_qf_owner,on_lookup,d);
#else
//	oop_adns_submit(adns,hostname,adns_r_a,adns_qf_owner,on_lookup,d);
#endif
}

void end(int)
{
	Plugin::deinit();
	Hub::killAll();
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

	EventHandler::init();

	ios::sync_with_stdio();
	Logs::stat << "starting " PACKAGE_NAME "/" PACKAGE_VERSION << endl;

	//do this here so we don't wind up doing extra
	//work if all they want is --version or --help
	Settings::parseArgs(argc, argv);

	Settings::readFromXML();

	//try loading
	Plugin::init();
	Plugin::openModule("loader");

	//Init random number generator
	srand(time(NULL));

/*	struct timer *timer = (struct timer*) malloc(sizeof(struct timer));
	gettimeofday(&timer->tv, NULL);
	timer->delay = 1000000;
	on_timer(src, timer->tv, timer);
*/

	EventHandler::mainLoop();

	return EXIT_SUCCESS;
}
