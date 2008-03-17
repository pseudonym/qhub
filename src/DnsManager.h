#ifndef QHUB_DNSMANAGER_H
#define QHUB_DNSMANAGER_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Singleton.h"
#include "EventManager.h"
#include <ares.h>
#include <vector>
#include <string>

namespace qhub {

class DnsListener;

class DnsManager : public Singleton<DnsManager>, public EventListener {
public:
	/*
	 * NOTE: do NOT call these two from inside a constructor, as the callback could
	 * be immediate and be called on an incomplete object.  Instead, add a timer event
	 * for 0 seconds and call these from inside that callback.
	 */
	void lookupName(const std::string&, DnsListener*) throw();
	//TODO add non-string version, but ok now b/c Socket returns strings
	void lookupAddr(const std::string&, DnsListener*) throw();

	// EventListener calls
	void onRead(int fd) throw();
	void onWrite(int fd) throw();
	void onTimer(int fd) throw();
private:
	friend class Singleton<DnsManager>;

	ares_channel chan;

	static void sockCallback(void* arg, int fd, int read, int write);
	static void lookupCallback(void* arg, int status, int timeouts, struct hostent* host);

	DnsManager() throw();
	~DnsManager() throw() {}
};

class DnsListener {
public:
	virtual void onResult(const std::string&, const std::vector<in_addr>&) throw() {}
#ifdef ENABLE_IPV6
	virtual void onResult(const std::string&, const std::vector<in6_addr>&) throw() {}
#endif
	virtual void onFailure() throw() {}

	virtual ~DnsListener() throw() {} // stfu, g++
};

} // namespace qhub

#endif // QHUB_DNSMANAGER_H
