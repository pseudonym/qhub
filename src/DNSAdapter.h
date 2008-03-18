#ifndef QHUB_DNSADAPTER_H
#define QHUB_DNSADAPTER_H

#include "EventManager.h"
#include <string>
#include <ares.h>

/*
 * This is deprecated and will be removed soon
 */

namespace qhub {

//abstract/portable DNS lookup/completion notification
class DNSAdapter: public EventListener {
public:
	DNSAdapter(const std::string& hostname);
	virtual ~DNSAdapter() throw();
	
	static void init();

	virtual void complete(const std::string& result) = 0;
	
protected:
	virtual void onRead(int) throw();
	virtual void onWrite(int) throw();
	virtual void onTimer(int) throw();
	
	bool doHack();
	
	ares_channel channel;
	std::string query;
	int fd;
};

}

#endif
