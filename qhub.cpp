#include "config.h"
#include "qhub.h"
#include "MyTime.h"

#include <stdio.h>

#include "ServerSocket.h"

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

void *test(oop_source *src, int fd, oop_event ev, void* usr)
{
	char tmp[100];
	memset(tmp, 0, 100);
	if(read(fd, tmp, sizeof(tmp))<1){
		src->cancel_fd(src, fd, OOP_READ);
	};
	if(strlen(tmp)>0){
		tmp[strlen(tmp)-1] = 0;
	}
	
	fprintf(stdout, "looking up %s\n", tmp);
	fflush(stdout);

	oop_adns_query * qadns = oop_adns_submit(adns,NULL,tmp,adns_r_a,adns_qf_owner,on_lookup,0);
	return OOP_CONTINUE;
}

void *test2(oop_source *src, int fd, oop_event ev, void* usr)
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
	oop_source* src = oop_sys_source(system);
#else
	oop_source* src = oop_event_new();
	fprintf(stderr, "Using libevent %p\n", src);
#endif
	//Set up ADNS
	adns = oop_adns_new(src,(adns_initflags)0,NULL);
	src->on_fd(src, 0, OOP_READ, test, 0);

	ServerSocket* tmp = new ServerSocket(9000, 0);

	src->on_fd(src, tmp->getFd(), OOP_READ, test2, tmp);

#ifndef HAVE_LIBOOP_EVENT
	oop_sys_run(system);
#else
	event_dispatch();
#endif
	return 0;
}
