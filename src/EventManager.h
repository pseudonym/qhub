// vim:ts=4:sw=4:noet
#ifndef QHUB_EVENTMANAGER_H
#define QHUB_EVENTMANAGER_H

#include "Singleton.h"
#include "compat_hash_map.h"
#include <boost/tuple/tuple.hpp>
#include <event.h>

namespace qhub {

// silly forward declarations
class EventListener;

// should really be in a utility header, but only temporary
// FIXME
template<class T>
struct PtrHash {
	size_t operator()(T*const e) const
	{ return (size_t)e; }
};

/**
 * Structure for managing all events, essentially a wrapper around
 * libevent.  There must be only one EventListener monitoring each
 * file descriptor, but each EventListener can listen for events on
 * multiple fd's.  There can be only one timer per EventListener at
 * a time, though it can be reset as necessary.
 */
class EventManager : public Singleton<EventManager> {
public:
	int run() throw();

	void enableRead(int fd, EventListener*) throw();
	void disableRead(int fd) throw();
	void enableWrite(int fd, EventListener*) throw();
	void disableWrite(int fd) throw();

	void addSignal(int sig, EventListener*) throw();
	void removeSignal(int sig) throw();

	// 'what' is so the class can determine why the timer expired
	// EventManager does nothing with it, so it can default to 0
	// if the timer is only used for one thing.
	void addTimer(EventListener*, int what=0, int secs=0, int micros=0) throw();
	void removeTimer(EventListener*) throw();

private:
	friend class Singleton<EventManager>;

	EventManager() throw();
	~EventManager() throw() {}

	// tables of active events
	// (yes, there is some duplication with libevent, but
	// we need access to this information)
	std::hash_map<int, boost::tuple<event,EventListener*> > reads;
	std::hash_map<int, boost::tuple<event,EventListener*> > writes;
	std::hash_map<int, boost::tuple<event,EventListener*> > signals;
	std::hash_map<EventListener*, boost::tuple<event,int>, PtrHash<EventListener> > timers;

	// callback functions (members because they might need to
	// access tables as part of dispatch process)
	static void readCallback(int fd, short ev, void* arg);
	static void writeCallback(int fd, short ev, void* arg);
	static void signalCallback(int sig, short ev, void* arg);
	static void timerCallback(int/* -unused- */, short ev, void* arg);
};


/*
 * Interface used by EventManager.  It's here because anybody including
 * one will want the other.
 */
class EventListener {
public:
	// we want default no-action because we can, unlike stupid Java interfaces.
	virtual void onRead(int fd) throw() {}
	virtual void onWrite(int fd) throw() {}
	virtual void onSignal(int sig) throw() {}
	virtual void onTimer(int what) throw() {}

	virtual ~EventListener() throw() {} // to shut the compiler up :)
};

} // namespace qhub

#endif // QHUB_EVENTMANAGER_H
