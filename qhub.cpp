#include "config.h"
#include "qhub.h"
#include "MyTime.h"

#include <stdio.h>

extern "C" {
#include <oop.h>
#include <oop-read.h>

#include <adns.h>
#include <oop-adns.h>
#ifdef HAVE_LIBEVENT 
#include <oop-event.h>
#endif
}

#ifdef HAVE_LIBEVENT
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
	for(int i=0; i<1000; i++){
		qadns = oop_adns_submit(adns,NULL,"slashdot.org",adns_r_a,adns_qf_owner,on_lookup,0);
		qadns = oop_adns_submit(adns,NULL,"g.mp",adns_r_a,adns_qf_owner,on_lookup,0);
		qadns = oop_adns_submit(adns,NULL,"cnn.com",adns_r_a,adns_qf_owner,on_lookup,0);
		qadns = oop_adns_submit(adns,NULL,"lartc.org",adns_r_a,adns_qf_owner,on_lookup,0);
		qadns = oop_adns_submit(adns,NULL,"mobile-burn.com",adns_r_a,adns_qf_owner,on_lookup,0);
		qadns = oop_adns_submit(adns,NULL,"aftonbladet.se",adns_r_a,adns_qf_owner,on_lookup,0);
		qadns = oop_adns_submit(adns,NULL,"e.r",adns_r_a,adns_qf_owner,on_lookup,0);
		qadns = oop_adns_submit(adns,NULL,"g.f",adns_r_a,adns_qf_owner,on_lookup,0);
		qadns = oop_adns_submit(adns,NULL,"w.s",adns_r_a,adns_qf_owner,on_lookup,0);
	}
	return OOP_CONTINUE;
}

int main()
{
#ifdef HAVE_LIBEVENT
	event_init();
#endif

#ifndef HAVE_LIBEVENT
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

#ifndef HAVE_LIBEVENT
	oop_sys_run(system);
#else
	event_dispatch();
#endif
	return 0;
}
