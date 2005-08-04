#include "EventHandler.h"
#include "Util.h"
#include "error.h"

#include <string>
#include <assert.h>

using namespace std;
using namespace qhub;

EventHandler::EventHandler() : ev(NULL), enabledFlags(0), fd(0)
{
	
}

EventHandler::~EventHandler() throw()
{
	if(ev != NULL){
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
	//remove the pertinent flag
	enabledFlags &= ~e;

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
	event_init();	
}

void EventHandler::mainLoop() throw()
{
	event_dispatch();
}
