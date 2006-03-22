#ifndef __DNSADAPTER_H_
#define __DNSADAPTER_H_

#include "EventHandler.h"
#include <string>
#include <ares.h>

namespace qhub {

//abstract/portable DNS lookup/completion notification
class DNSAdapter: public EventHandler {
public:
	DNSAdapter(const std::string& hostname);
	virtual ~DNSAdapter() throw();
	
	static void init();

	virtual void complete(const std::string& result) = 0;
	
protected:
	virtual bool onRead() throw();
	virtual void onWrite() throw();
	virtual void onTimeout() throw();
	
	bool doHack();
	
	ares_channel channel;
	std::string query;
};


}


#endif
