#include "EventHandler.h"
#include "Util.h"

#include <string>
#include <assert.h>

using namespace std;
using namespace qhub;

EventHandler::EventHandler() : ev(NULL), enabledFlags(0)
{
	
}

EventHandler::~EventHandler() throw()
{
	Util::log("~EventHandler\n");
	if(ev != NULL){
		Util::log(Util::format("Deleting EventHandler &p\n", this));
		event_del(ev);
	}
	free(ev);
}

void demux(int fd, short e, void* event)
{
	EventHandler* eh = (EventHandler*) event;

	assert(eh->getEnabledFlags() != 0 && "We are not supposed to be receiving events!");

	Util::log(Util::format("Demux EventHandler=%p fd=%d event=%d\n", (int)eh, fd, e));		
	
	bool result = true;
	if(e & EV_READ){
		result = eh->onRead();
	}
	//we do not want to call this one if we are already deleted (delete this)
	if(e & EV_WRITE && result){
		eh->onWrite();
	}
}

void EventHandler::disableMe(event e) throw()
{
	Util::log(Util::format("Disabling socket %d for %d, enabledFlags=%d\n", fd, e, enabledFlags), 10);

	//remove the pertinent flag
	enabledFlags &= ~e;

	Util::log(Util::format("enabledFlags=%d\n", enabledFlags));

	event_del(ev);
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
		event_add(ev, NULL);
	}
}

void EventHandler::enableMe(event e) throw()
{
	Util::log(Util::format("Enabling socket %d for %d\n", fd, e), 10);
	
	if(enabledFlags != 0){
		event_del(ev);
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
	event_add(ev, NULL);
}

void EventHandler::init() throw()
{
	Util::log("Initializing event handling.\n");

	event_init();	
}

void EventHandler::mainLoop() throw()
{
	Util::log("Entering mainloop.\n");

	event_dispatch();
}
