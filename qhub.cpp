#include "config.h"
#include "qhub.h"

#include <stdio.h>

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
	//fprintf(stdout, "%s =>",reply->owner);
	//fflush(stdout);

	DNSUser* d = (DNSUser*) data;

	d->onLookup(reply);

	/*if (adns_s_ok != reply->status) {
		printf(" error: %s\n",adns_strerror(reply->status));
	} else {
		if (NULL != reply->cname) {
			printf(" (%s)",reply->cname);
		}
		assert(adns_r_a == reply->type);
		for (int i = 0; i < reply->nrrs; ++i) {
			printf(" %d: %s",i, inet_ntoa(reply->rrs.inaddr[i]));
		}
		printf("\n");
	}*/

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

void qhub::lookup(const char* hostname, DNSUser* d){
	//oop_adns_query * qadns = oop_adns_submit(adns,NULL,hostname,adns_r_a,adns_qf_owner,on_lookup,d);
	//this might be required to compile
	//oop_adns_query * qadns = oop_adns_submit(adns,hostname,adns_r_a,adns_qf_owner,on_lookup,d);
#if 0
	1.0:

	oop_adns_query *oop_adns_submit_reverse(
	oop_adapter_adns *,int *errcode,
	const struct sockaddr *addr,adns_rrtype type,adns_queryflags flags,
	oop_adns_call *,void *);

	0.8:

	oop_adns_query *oop_adns_submit(
	oop_adapter_adns *,
	const char *owner,adns_rrtype type,adns_queryflags flags,
	oop_adns_call *,void *);

	0.9:

oop_adns_query *oop_adns_submit(
	oop_adapter_adns *,
	const char *owner,adns_rrtype type,adns_queryflags flags,
	oop_adns_call *,void *);

	Mine:

	oop_adns_query *oop_adns_submit(
        oop_adapter_adns *,int *errcode,
        const char *owner,adns_rrtype type,adns_queryflags flags,
        oop_adns_call *,void *);


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
	adns = oop_adns_new(src, (adns_initflags)0, NULL);

	//try loading
	Plugin::init();
	Plugin::openModule("loader");

	Settings::readFromXML();

	//Init random number generator
	srand(time(NULL));

        struct timer *timer = (struct timer*) malloc(sizeof(struct timer));
        gettimeofday(&timer->tv, NULL);
        timer->delay = 1000000;
        on_timer(src, timer->tv, timer);


#ifndef HAVE_LIBOOP_EVENT
	oop_sys_run(system);
#else
	event_dispatch();
#endif

	return 0;
}
