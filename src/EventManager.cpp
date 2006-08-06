// vim:ts=4:sw=4:noet
#include "EventManager.h"
#include "Logs.h"

using namespace std;
using namespace qhub;
using boost::get;


EventManager::EventManager() throw()
{
	if(event_init()) {
		Logs::stat << "initialized libevent/" << event_get_version() << " using method "
				<< event_get_method() << endl;
	} else {
		Logs::err << "failed to initialize libevent: FATAL" << endl;
		exit(EXIT_FAILURE);
	}
}

int EventManager::run() throw()
{
	return event_dispatch();
}

////////////////
// add/remove //
////////////////

void EventManager::enableRead(int fd, EventListener* eh) throw()
{
	// have to remove before we re-add...
	disableRead(fd);

	reads[fd] = boost::make_tuple(event(), eh);
	event& ev = get<0>(reads[fd]);
	event_set(&ev, fd, EV_READ | EV_PERSIST, readCallback, eh);
	event_add(&ev, NULL);
}

void EventManager::disableRead(int fd) throw()
{
	if(!reads.count(fd))
		return;

	event_del(&get<0>(reads[fd]));
	reads.erase(fd);
}

void EventManager::enableWrite(int fd, EventListener* eh) throw()
{
	disableWrite(fd);

	writes[fd] = boost::make_tuple(event(), eh);
	event& ev = get<0>(writes[fd]);
	event_set(&ev, fd, EV_WRITE | EV_PERSIST, writeCallback, eh);
	event_add(&ev, NULL);
}

void EventManager::disableWrite(int fd) throw()
{
	if(!writes.count(fd))
		return;

	event_del(&get<0>(writes[fd]));
	writes.erase(fd);
}

void EventManager::addSignal(int sig, EventListener* eh) throw()
{
	removeSignal(sig);

	signals[sig] = boost::make_tuple(event(), eh);
	event& ev = get<0>(signals[sig]);
	signal_set(&ev, sig, signalCallback, eh);
	signal_add(&ev, NULL);
}

void EventManager::removeSignal(int sig) throw()
{
	if(!signals.count(sig))
		return;

	signal_del(&get<0>(signals[sig]));
	signals.erase(sig);
}

void EventManager::addTimer(EventListener* eh, int what, int secs, int micros) throw()
{
	removeTimer(eh);

	timers[eh] = boost::make_tuple(event(), what);
	event& ev = get<0>(timers[eh]);
	evtimer_set(&ev, timerCallback, eh);
	timeval t;
	t.tv_sec = secs;
	t.tv_usec = micros;
	evtimer_add(&ev, &t);
}

void EventManager::removeTimer(EventListener* eh) throw()
{
	if(!timers.count(eh))
		return;

	evtimer_del(&get<0>(timers[eh]));
	timers.erase(eh);
}

///////////////
// callbacks //
///////////////

void EventManager::readCallback(int fd, short ev, void* arg)
{
	assert(ev == EV_READ && instance()->reads.count(fd));

	EventListener* eh = static_cast<EventListener*>(arg);
	eh->onRead(fd);
}

void EventManager::writeCallback(int fd, short ev, void* arg)
{
	assert(ev == EV_WRITE && instance()->writes.count(fd));

	EventListener* eh = static_cast<EventListener*>(arg);
	eh->onWrite(fd);
}

void EventManager::signalCallback(int sig, short ev, void* arg)
{
	assert(ev == EV_SIGNAL && instance()->signals.count(sig));

	EventListener* eh = static_cast<EventListener*>(arg);
	eh->onSignal(sig);
}

void EventManager::timerCallback(int fd, short ev, void* arg)
{
	assert(ev == EV_TIMEOUT && fd == -1);

	EventListener* eh = static_cast<EventListener*>(arg);
	assert(instance()->timers.count(eh));
	eh->onTimer(instance()->timers[eh].get<1>());

	// timers are not persistent, so libevent will remove it.
	// we need to as well
	instance()->timers.erase(eh);
}

