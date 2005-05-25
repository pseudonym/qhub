#ifndef __DNSUSER_H_
#define __DNSUSER_H_

#include <adns.h>
#include "qhub.h"

namespace qhub {

//abstract/portable DNS lookup/completion notification
class DNSUser {
public:
	//XXX: need to get rid of ADNS here.
	virtual void onLookup() = 0;

	void lookup(const char* hostname) { qhub::lookup(hostname, this); };
};


}


#endif
