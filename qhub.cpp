#include "config.h"
#include "qhub.h"
#include "MyTime.h"

#include <stdio.h>

#include "DNSUser.h"
#include "Hub.h"


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

static void *on_lookup(oop_adapter_adns *adns,adns_answer *reply,void *data)
{
	fprintf(stdout, "%s =>",reply->owner);
	fflush(stdout);

	DNSUser* d = (DNSUser*) data;

	if (adns_s_ok != reply->status) {
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
	}

	return OOP_CONTINUE;
}

void *fd_demux(oop_source *src, int fd, oop_event ev, void* usr)
{
	Socket* s = (Socket*) usr;

	switch(ev){
		case OOP_READ:
			s->on_read();
			break;
		case OOP_WRITE:
			s->on_write();
			break;
	}

	return OOP_CONTINUE;
}

static oop_source* src;

void qhub::enable(int fd, oop_event ev, Socket* s)
{
	fprintf(stderr, "Enabling fd %d %d\n", fd, ev);
	src->on_fd(src, fd, ev, fd_demux, s);
}

void qhub::cancel(int fd, oop_event ev)
{
	fprintf(stderr, "Cancellng fd %d %d\n", fd, ev);
	src->cancel_fd(src, fd, ev);
}

void qhub::lookup(const char* hostname, DNSUser* d){
	oop_adns_query * qadns = oop_adns_submit(adns,NULL,hostname,adns_r_a,adns_qf_owner,on_lookup,d);
}

int main()
{
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
	adns = oop_adns_new(src,(adns_initflags)0,NULL);

	Hub* tmp = new Hub();

#ifndef HAVE_LIBOOP_EVENT
	oop_sys_run(system);
#else
	event_dispatch();
#endif

	delete tmp;
	return 0;
}
