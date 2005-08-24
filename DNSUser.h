#ifndef __DNSUSER_H_
#define __DNSUSER_H_

#include <adns.h>
#include "qhub.h"

namespace qhub {

//abstract/portable DNS lookup/completion notification
class DNSUser: public EventHandler {
public:
	static void init();

	virtual bool onRead() throw() = 0;
	virtual void onWrite() throw() = 0;
		       
};


}


#endif
