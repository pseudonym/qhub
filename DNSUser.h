#ifndef __DNSUSER_H_
#define __DNSUSER_H_

#include <adns.h>

namespace qhub {

class DNSUser {
public:
	virtual void onLookup(adns_answer *reply) const = 0;
};


}


#endif
