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

	//this is needed for timeout-support
	virtual void onTimeout() throw() = 0;
	virtual bool onRead() throw() = 0;
	virtual void onWrite() throw() = 0;

	enum type { ev_none=0, ev_read=1, ev_write=2, READ=ev_read, WRITE=ev_write };

	void enableMe(type e, int secs = -1, int micros = 0) throw();
	void disableMe(type e) throw();

	int getFd() throw() { return fd; };
	
	static void init() throw();

	static void mainLoop() throw();

	int getEnabledFlags() { return enabledFlags; };
protected:
	int fd;
private:
	void initEvents() throw();
	bool inited;
	struct event read_ev;
	struct event write_ev;
	int enabledFlags;
};

}

#endif
