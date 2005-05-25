#ifndef __EVENT_HANDLER_H_
#define __EVENT_HANDLER_H_

typedef unsigned char u_char;
#include <sys/time.h>
#include <event.h>

namespace qhub {

//handle events on filedescriptors
class EventHandler {
public:
	EventHandler();
	virtual ~EventHandler() throw();

	virtual bool onRead() throw() = 0;
	virtual void onWrite() throw() = 0;

	enum type { ev_read=1, ev_write=2
	};

	void enableMe(type e) throw();
	void disableMe(type e) throw();

	int getSocket() throw() { return fd; };
	
	static void init() throw();

	static void mainLoop() throw();

	struct ::event* getEventStruct() throw() { return ev; };
	int getEnabledFlags() { return enabledFlags; };
protected:
	int fd;
private:
	struct ::event *ev;
	int enabledFlags;
};

}

#endif
