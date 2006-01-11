#ifndef __DNSADAPTER_H_
#define __DNSADAPTER_H_

#include <string>
extern "C" {
#include <ares.h>
}
#include "qhub.h"

using namespace std;

namespace qhub {

//abstract/portable DNS lookup/completion notification
class DNSAdapter: public EventHandler {
public:
	DNSAdapter(const string& hostname);
	virtual ~DNSAdapter() throw();
	
	static void init();

	virtual void complete(const string& result) = 0;
	
protected:
	virtual bool onRead() throw();
	virtual void onWrite() throw();
	virtual void onTimeout() throw();
	
	bool doHack();
	
	ares_channel channel;
	string query;
};


}


#endif
