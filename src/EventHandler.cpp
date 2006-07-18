#include "EventHandler.h"
#include "Util.h"
#include "error.h"
#include "Logs.h"

#include <string>
#include <cassert>

using namespace std;
using namespace qhub;

EventHandler::EventHandler() : fd(-1), inited(false), enabledFlags(0)
{
	
}

EventHandler::~EventHandler() throw()
{
	disableMe(READ);
	disableMe(WRITE);
}

namespace { // don't want these polluting the global namespace

void readCallback(int fd, short e, void* arg)
{
	EventHandler* eh = static_cast<EventHandler*>(arg);

	assert((eh->getEnabledFlags() & EventHandler::READ) && "received read event when disabled");

	// if we timeout, we probably don't care about the read,
	// so let it fire next time if we still want it...
	if(e & EV_TIMEOUT)
		eh->onTimeout();
	else
		eh->onRead();
}

void writeCallback(int fd, short e, void* arg)
{
	EventHandler* eh = static_cast<EventHandler*>(arg);

	assert((eh->getEnabledFlags() & EventHandler::WRITE) && "received write event when disabled");

	// see above
	if(e & EV_TIMEOUT)
		eh->onTimeout();
	else
		eh->onWrite();
}

void genericCallback(int fd, short e, void* arg)
{
	assert(fd == -1 && e == EV_TIMEOUT);

	
}

} //namespace

void EventHandler::disableMe(type e) throw()
{
	//remove the pertinent flag
	enabledFlags &= ~e;

	//the interface to libevent is somewhat "weird":
	//We must delete the entire event, even if it still
	//has flags we want to monitor, and the re-add it, 
	//instead of just signalling the changes...
	if((e & READ) && event_del(&read_ev) == -1) {
		Logs::err << "EventHandler::disableMe: deleting fd " << getFd() << " for read failed\n";
		exit(3);
	}
	if((e & WRITE) && event_del(&write_ev) == -1) {
		Logs::err << "EventHandler::disableMe: deleting fd " << getFd() << " for write failed\n";
		exit(4);
	}
}

void EventHandler::enableMe(type e, int secs, int micros) throw()
{
	initEvents();

	struct timeval* t;
	if(secs == -1) {
		t = NULL;
	} else {
		t = new struct timeval;
		t->tv_sec = secs;
		t->tv_usec = micros;
	}

	if((e & READ) && event_add(&read_ev, t) == -1) {
		Logs::err << "EventHandler::enableMe: adding fd " << getFd() << " for read failed\n";
		exit(5);
	}
	if((e & WRITE) && event_add(&write_ev, t) == -1) {
		Logs::err << "EventHandler::enableMe: adding fd " << getFd() << " for write failed\n";
		exit(6);
	}

	enabledFlags |= e;
	delete t; // if NULL, no-op
}

void EventHandler::initEvents() throw()
{
	if(inited)
		return;

	event_set(&read_ev, fd, EV_READ | EV_PERSIST, readCallback, this);
	event_set(&write_ev, fd, EV_WRITE | EV_PERSIST, writeCallback, this);
	inited = true;
}

/*
void EventHandler::addTimer(void (*func)(), int secs, int micros) throw()
{
	struct timeval t;
	t.tv_sec = secs;
	t.tv_usec = micros;

	event_once(-1, EV_TIMEOUT, &t);

	delete t;
}*/

void EventHandler::init() throw()
{
	event_init();	
	
	Logs::stat << "Started Libevent/" << event_get_version() << " using method "
			<< event_get_method() << endl;
}

void EventHandler::mainLoop() throw()
{
	event_dispatch();
}
