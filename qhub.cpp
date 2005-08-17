#include "config.h"
#include "qhub.h"
#include "error.h"

#include <stdio.h>

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

#include <adns.h>
#include <oop-adns.h>
#ifdef HAVE_LIBOOP_EVENT 
#include <oop-event.h>
#endif
}

#ifdef HAVE_LIBOOP_EVENT
#include <event.h>
#endif

using namespace qhub;

static oop_adapter_adns *adns;
static oop_source* src;

struct timer {
	struct timeval tv;
	int delay;
};

static void *on_lookup(oop_adapter_adns *adns, adns_answer *reply, void *data)
{
	DNSUser* d = (DNSUser*) data;

	d->onLookup(reply);

	return OOP_CONTINUE;
}

static void *on_timer(oop_source *source, struct timeval tv, void *data) {
	struct timer *timer = (struct timer*)data;
	timer->tv = tv;
	timer->tv.tv_usec += timer->delay;
	while(timer->tv.tv_usec >= 1000000) {
		timer->tv.tv_sec++;
		timer->tv.tv_usec -= 1000000;
	}
	source->on_time(source, timer->tv, on_timer, data);
	Timer::tick();
	return OOP_CONTINUE;
}

void *fd_demux(oop_source *src, int fd, oop_event ev, void* usr)
{
	Socket* s = (Socket*) usr;

	switch(ev){
	case OOP_READ:
		s->onRead();
		break;
	case OOP_WRITE:
		s->onWrite();
		break;
	case OOP_EXCEPTION:
	case OOP_NUM_EVENTS:
	default:
		break;
	}

	return OOP_CONTINUE;
}

void qhub::enable_fd(int fd, oop_event ev, Socket* s)
{
	src->on_fd(src, fd, ev, fd_demux, s);
}

void qhub::cancel_fd(int fd, oop_event ev)
{
	src->cancel_fd(src, fd, ev);
}

void qhub::lookup(const char* hostname, DNSUser* const d){
#ifdef LIBOOP_RECENT
	oop_adns_submit(adns,NULL,hostname,adns_r_a,adns_qf_owner,on_lookup,d);
#else
	oop_adns_submit(adns,hostname,adns_r_a,adns_qf_owner,on_lookup,d);
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


	//do this here so we don't wind up doing extra
	//work if all they want is --version or --help
	Settings::parseArgs(argc, argv);

	Logs::stat << "starting " PACKAGE_NAME "/" PACKAGE_VERSION << endl;

#ifdef HAVE_LIBOOP_EVENT
	event_init();
#endif

#ifndef HAVE_LIBOOP_EVENT
	oop_source_sys* system;

	if((system=oop_sys_new()) == NULL){
		Logs::err << "Malloc failure.\n";
		exit(EXIT_FAILURE);
	}
	src = oop_sys_source(system);
	Logs::stat << "Using liboop system event source: select() will be used.\n";
#else
	src = oop_event_new();
	Logs::stat << "Using libevent source adapter\n";
#endif
	//Set up ADNS
	adns = oop_adns_new(src, (adns_initflags)0, NULL);

	Settings::readFromXML();

	//try loading
	Plugin::init();
	Plugin::openModule("loader");

	//Init random number generator
	srand(time(NULL));

	struct timer *timer = new struct timer;
	gettimeofday(&timer->tv, NULL);
	timer->delay = 1000000;
	on_timer(src, timer->tv, timer);


#ifndef HAVE_LIBOOP_EVENT
	oop_sys_run(system);
#else
	event_dispatch();
#endif

	return EXIT_SUCCESS;
}
