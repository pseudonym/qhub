#ifndef QHUB_DNSMANAGER_H
#define QHUB_DNSMANAGER_H

#include "Singleton.h"
#include "EventManager.h"
#include <ares.h>
#include <vector>
#include <string>

namespace qhub {

class DnsListener;

class DnsManager : public Singleton<DnsManager>, public EventListener {
public:
	void lookupName(const std::string&, DnsListener*) throw();
	//TODO add non-string version, but ok now b/c Socket returns strings
	void lookupIp(const std::string&, DnsListener*) throw();

	// EventListener calls
	void onRead(int fd) throw();
	void onWrite(int fd) throw();
	void onTimer(int fd) throw();
private:
	friend class Singleton<DnsManager>;

	ares_channel chan;

	static void sockCallback(void* arg, int fd, int read, int write);
	static void lookupCallback(void* arg, int status, struct hostent* host);

	DnsManager() throw();
	~DnsManager() throw() {}
};

class DnsListener {
public:
	virtual void onResult(const std::string&, const std::vector<in_addr>&) throw() {}
	virtual void onResult(const std::string&, const std::vector<in6_addr>&) throw() {}
	virtual void onFailure() throw() {}

	virtual ~DnsListener() throw() {} // stfu, g++
};

} // namespace qhub

#endif // QHUB_DNSMANAGER_H
