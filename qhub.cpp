#include "config.h"
#include "qhub.h"
#include "error.h"

#include <stdio.h>

#include "EventHandler.h"
#include "DNSAdapter.h"
#include "Hub.h"
#include "Timer.h"

#include "Plugin.h"
#include "Settings.h"
#include "Util.h"
#include "Logs.h"

#include <string>

using namespace std;

using namespace qhub;

struct timeInfo {
	struct timeval tv;
	int delay;
	struct event* ev;
};

void end(int)
{
	Plugin::deinit();
	Hub::killAll();
	Settings::save();
	exit(EXIT_SUCCESS);
}

void timerCallback(int, short int, void *ev)
{
	timeInfo* tmp = (timeInfo*) ev;

	Timer::tick();
	
	evtimer_add(tmp->ev, &tmp->tv); 
}

class majs: public DNSAdapter
{
public:
	majs(string s) : DNSAdapter(s) {} 
	
	virtual void complete(string s) { Logs::stat << "complete: " << s << endl; }
};

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
	DNSAdapter::init();

	majs* p  = new majs("www.google.com");
	
	Logs::stat << "starting " PACKAGE_NAME "/" PACKAGE_VERSION << endl;

	//do this here so we don't wind up doing extra
	//work if all they want is --version or --help
	Settings::parseArgs(argc, argv);

	Settings::load();

	//try loading
	Plugin::init();
	Plugin::openModule("loader");

	//Init random number generator
	srand(time(NULL));

	struct timeInfo *tmp = new struct timeInfo;
	tmp->ev = new struct event;
	
	evtimer_set(tmp->ev, timerCallback, tmp);
	
	tmp->tv.tv_usec=0;
	tmp->tv.tv_sec=1;
	tmp->delay = 1;	
	
	evtimer_add(tmp->ev, &tmp->tv);
	
	EventHandler::mainLoop();

	return EXIT_SUCCESS;
}
