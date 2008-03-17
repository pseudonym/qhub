#include "DnsManager.h"
#include "Logs.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;
using namespace qhub;

DnsManager::DnsManager() throw()
{
	ares_options opt;

	opt.sock_state_cb = sockCallback;
	opt.sock_state_cb_data = this; // doesn't really matter what this is...

	int ret = ares_init_options(&chan, &opt, ARES_OPT_SOCK_STATE_CB);
	if(ret == ARES_SUCCESS) {
		Logs::stat << "initialized c-ares for DNS resolution" << endl;
	} else {
		Logs::err << "c-ares initialization failed: " << ares_strerror(ret) << endl;
		exit(EXIT_FAILURE);
	}
}

void DnsManager::lookupName(const string& name, DnsListener* arg) throw()
{
	// should really try INET6, but not really used...
	ares_gethostbyname(chan, name.c_str(), AF_INET, lookupCallback, arg);
	timeval tv;
	if(ares_timeout(chan, NULL, &tv)) // NULL if no timeout
		EventManager::instance()->addTimer(this, 0, tv.tv_sec, tv.tv_usec);
}

void DnsManager::lookupAddr(const string& ip, DnsListener* arg) throw()
{
	in_addr addr;
#ifdef ENABLE_IPV6
	in6_addr addr6;
#endif
	if(inet_pton(AF_INET, ip.c_str(), &addr)) {
		ares_gethostbyaddr(chan, &addr, sizeof(in_addr), AF_INET, lookupCallback, arg);
#ifdef ENABLE_IPV6
	} else if(inet_pton(AF_INET6, ip.c_str(), &addr6)) {
		ares_gethostbyaddr(chan, &addr6, sizeof(in6_addr), AF_INET6, lookupCallback, arg);
#endif
	} else {
		assert(0 && "invalid address given to DnsManager::lookupIp()");
	}
	timeval tv;
	if(ares_timeout(chan, NULL, &tv)) // NULL if no timeout
		EventManager::instance()->addTimer(this, 0, tv.tv_sec, tv.tv_usec);
}

/////////////////////////
// EventListener calls //
/////////////////////////

void DnsManager::onRead(int fd) throw()
{
	fd_set r, w;

	FD_ZERO(&r);
	FD_ZERO(&w);

	FD_SET(fd, &r);

	// ares interface for this is fugly, but what can you do?
	ares_process(chan, &r, &w);
}

void DnsManager::onWrite(int fd) throw()
{
	fd_set r, w;

	FD_ZERO(&r);
	FD_ZERO(&w);

	FD_SET(fd, &w);

	ares_process(chan, &r, &w);
}

void DnsManager::onTimer(int what) throw()
{
	fd_set r, w;

	FD_ZERO(&r);
	FD_ZERO(&w);

	ares_process(chan, &r, &w);
}

//////////////////////
// static callbacks //
//////////////////////

void DnsManager::sockCallback(void* arg, int fd, int read, int write)
{
	assert(arg == instance());

	if(read)
		EventManager::instance()->enableRead(fd, instance());
	else
		EventManager::instance()->disableRead(fd);

	if(write)
		EventManager::instance()->enableWrite(fd, instance());
	else
		EventManager::instance()->disableWrite(fd);
}

void DnsManager::lookupCallback(void* arg, int status, int timeouts, struct hostent* host)
{
	DnsListener* dl = static_cast<DnsListener*>(arg);

	if(status != ARES_SUCCESS) {
		Logs::err << "DNS lookup failed: " << ares_strerror(status) << endl;
		dl->onFailure();
	} else if(host->h_addrtype == AF_INET) {
		// IPv4
		assert(host->h_length == sizeof(in_addr));
		vector<in_addr> addrs;
		for(char** p = host->h_addr_list; *p; p++)
			addrs.push_back(*reinterpret_cast<in_addr*>(*p)); // nasty casting gogo!
		dl->onResult(host->h_name, addrs);
#ifdef ENABLE_IPV6
	} else if(host->h_addrtype == AF_INET6) {
		// IPv6
		assert(host->h_length == sizeof(in6_addr));
		vector<in6_addr> addrs;
		for(char** p = host->h_addr_list; *p; p++)
			addrs.push_back(*reinterpret_cast<in6_addr*>(*p));
		dl->onResult(host->h_name, addrs);
#endif
	} else {
		Logs::stat << "Lookup of " << host->h_name
				<< " returned unknown address type, ignoring" << endl;
	}
}

