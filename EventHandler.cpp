#include "EventHandler.h"
#include "Util.h"
#include "error.h"

#include <string>
#include <assert.h>

using namespace std;
using namespace qhub;

EventHandler::EventHandler() : ev(NULL), enabledFlags(0)
{
	
}

EventHandler::~EventHandler() throw()
{
	log(qstat, "~EventHandler\n");
	if(ev != NULL){
		log(qstat, "Deleting EventHandler\n");
		if(event_del(ev) == -1){
			exit(1);	
		}
	}
	free(ev);
}

void demux(int fd, short e, void* event)
{
	EventHandler* eh = (EventHandler*) event;

	assert(eh->getEnabledFlags() != 0 && "We are not supposed to be receiving events!");

	log(qstat, format("Demux EventHandler=%p fd=%d event=%d\n") % eh % fd % e);		

	bool result = true;
	if(e & EV_READ){
		result = eh->onRead();
	}
	//we do not want to call this one if we are already deleted (delete this)
	if(e & EV_WRITE && result){
		eh->onWrite();
	}
}

void EventHandler::disableMe(type e) throw()
{
	log(qstat, format("Disabling socket %d for %d, enabledFlags=%d\n") % fd % e % enabledFlags);

	//remove the pertinent flag
	enabledFlags &= ~e;

	log(qstat, format("enabledFlags=%d\n") % enabledFlags);

	if(event_del(ev) == -1){
		exit(1);
	}
	if(enabledFlags == 0){
		free(ev);
		ev = NULL;
	} else {
		// we still want other events
		int use_flags = EV_PERSIST;
		if(enabledFlags & ev_read){
			use_flags |= EV_READ;
		}
		if(enabledFlags & ev_write){
			use_flags |= EV_WRITE;
		}
		event_set(ev, fd, use_flags, demux, this);
		if(event_add(ev, NULL) == -1){
			exit(1);
		}
	}
}

void EventHandler::enableMe(type e) throw()
{
	log(qstat, format("Enabling socket %d for %d\n") % fd % e);
	
	if(enabledFlags != 0){
		if(event_del(ev) == -1){
			exit(1);
		}
		assert(ev != NULL && "We are enabled yet have no event-struct!");
	}

	if(ev == NULL){
		ev = (struct ::event*) malloc(sizeof(struct ::event));
	}
	memset(ev, sizeof(struct ::event), 0);

	enabledFlags |= e;

	short use_event = EV_PERSIST;
	if(enabledFlags & ev_read){
		use_event |= EV_READ;
	}
	if(enabledFlags & ev_write){
		use_event |= EV_WRITE;
	}
	event_set(ev, fd, use_event, demux, this);
	if(event_add(ev, NULL) == -1){
		exit(1);
	}
}

void EventHandler::init() throw()
{
	log(qstat, "Initializing event handling.\n");

	event_init();	
}

void EventHandler::mainLoop() throw()
{
	log(qstat, "Entering mainloop.\n");

	event_dispatch();
}
