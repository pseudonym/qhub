#include "config.h"
#include "qhub.h"

#include <stdio.h>

#include "EventHandler.h"
#include "DNSUser.h"
#include "Hub.h"
#include "Timer.h"

#include "Plugin.h"
#include "Settings.h"

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
	exit(0);
}

int main()
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

#if 0
#ifdef HAVE_LIBOOP_EVENT
	event_init();
#endif

#ifndef HAVE_LIBOOP_EVENT
	oop_source_sys* system;

	if((system=oop_sys_new()) == NULL){
		fprintf(stderr, "Malloc failure.\n");
		exit(1);
	}
	src = oop_sys_source(system);
	fprintf(stderr, "Using liboop system event source: select() will be used.\n");
#else
	src = oop_event_new();
	fprintf(stderr, "Using libevent source adapter\n", src);
#endif
	//Set up ADNS
	//adns = oop_adns_new(src, (adns_initflags)0, NULL);
#endif //#if 0

	//try loading
	Plugin::init();
	Plugin::openModule("loader.so");

	Settings::readFromXML();

	//Init random number generator
	srand(time(NULL));

/*	struct timer *timer = (struct timer*) malloc(sizeof(struct timer));
	gettimeofday(&timer->tv, NULL);
	timer->delay = 1000000;
	on_timer(src, timer->tv, timer);
*/

#ifndef HAVE_LIBOOP_EVENT
//	oop_sys_run(system);
#else
//	event_dispatch();
#endif

	EventHandler::mainLoop();

	return 0;
}
