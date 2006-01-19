#include "EventHandler.h"
#include "Util.h"
#include "error.h"
#include "Logs.h"

#include <string>
#include <cassert>

using namespace std;
using namespace qhub;

EventHandler::EventHandler() : fd(-1), ev(NULL), enabledFlags(0)
{
	
}

EventHandler::~EventHandler() throw()
{
	if(ev != NULL){
		if(event_del(ev) == -1){
			exit(2);	
		}
	}
	delete ev;
}

void demux(int fd, short e, void* event)
{
	EventHandler* eh = (EventHandler*) event;

	assert(eh->getEnabledFlags() != 0 && "We are not supposed to be receiving events!");

	bool result = true;
	if(e & EV_TIMEOUT){
		eh->onTimeout();
	}
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

	//the interface to libevent is somewhat "weird":
	//We must delete the entire event, even if it still
	//has flags we want to monitor, and the re-add it, 
	//instead of just signalling the changes...
	if(event_del(ev) == -1){
		exit(3);
	}
	if(enabledFlags == 0){
		delete ev;
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
			exit(4);
		}
	}
}

void EventHandler::enableMe(type e, timeval* const timeout) throw()
{
	if(enabledFlags != 0){
		if(event_del(ev) == -1){
			exit(5);
		}
		assert(ev != NULL && "We are enabled yet have no event-struct!");
	}

	if(ev == NULL){
		ev = new struct ::event;
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
	if(event_add(ev, timeout) == -1){
		exit(1);
	}
}

void EventHandler::init() throw()
{
	event_init();	
	
	//Logs::stat << "Eventhandling using Libevent method " << event_get_method() << endl;
}

void EventHandler::mainLoop() throw()
{
	event_dispatch();
}
