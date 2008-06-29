// vim:ts=4:sw=4:noet
#include "Socket.h"

#include "Logs.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
using namespace qhub;

Socket::Socket(Domain d, int t, int p) throw(socket_error)
		: fd(-1), domain(d), ip4OverIp6(false),
		writeEnabled(false), written(0), disconnected(false)
{
	create();

	fd = ::socket(d, t, p);
	if(getFd() == -1){
		throw socket_error("Could not allocate socket: " + Util::errnoToString(errno));
	}

	setNonBlocking();
	setSendTimeout(2);
	initSocketNames();
}

Socket::Socket(int f, Domain d) throw()
		: domain(d), ip4OverIp6(false),
		writeEnabled(false), written(0), disconnected(false)
{
	fd = f;
	create();
	initSocketNames();
}

Socket::~Socket() throw()
{
	destroy();
}

void Socket::create() throw()
{
	if(domain == Socket::IP4) {
		saddrp = reinterpret_cast<struct sockaddr*>(new sockaddr_in);
		saddrl = sizeof(sockaddr_in);
		memset(saddrp, '\0', saddrl);
		inaddrp = &((struct sockaddr_in*)saddrp)->sin_addr;
		af = AF_INET;
		((struct sockaddr_in*)saddrp)->sin_family = af;
#ifdef ENABLE_IPV6
	} else if(domain == Socket::IP6) {
		saddrp = (struct sockaddr*)new sockaddr_in6;
		saddrl = sizeof(sockaddr_in6);
		memset(saddrp, '\0', saddrl);
		inaddrp = &((struct sockaddr_in6*)saddrp)->sin6_addr;
		af = AF_INET6;
		((struct sockaddr_in6*)saddrp)->sin6_family = af;
#endif    
	} else {
		assert(0);
	}
}

void Socket::destroy() throw()
{
	if(domain == Socket::IP4) {
		delete (struct sockaddr_in*)saddrp;
	}
#ifdef ENABLE_IPV6    
	else if(domain == Socket::IP6) {
		delete (struct sockaddr_in6*)saddrp;
	}
#endif
}

void Socket::connect(const string& ip, short port) throw(socket_error)
{
	sockaddr_in dest_addr;

	memset(&dest_addr, '\0', sizeof(sockaddr_in));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	if(dest_addr.sin_addr.s_addr == INADDR_NONE)
		throw socket_error(ip + " is not a valid IP address");

	if(!::connect(getFd(), reinterpret_cast<sockaddr*>(&dest_addr), sizeof(sockaddr))
			&& errno != EINPROGRESS) // ok if failed because of in progress
		throw socket_error("can't connect to " + ip + ": " + Util::errnoToString(errno));

	EventManager::instance()->enableRead(getFd(), this);
}

void Socket::listen(int backlog) throw(socket_error)
{
	if(::listen(fd, backlog) == -1)
		throw socket_error(Util::errnoToString(errno));
}

void Socket::bind(const string& a, short p) throw()
{
#ifdef HAVE_INET_PTON
	if(inet_pton(af, a.c_str(), inaddrp) == 0) {
		if(domain == Socket::IP4) {
			if(inet_pton(af, "0.0.0.0", inaddrp) == 0) {
				Logs::err << "error (pton+ipv4): setBoundAddress:inet_pton:0.0.0.0\n";
				exit(EXIT_FAILURE);
			}
		}
#ifdef ENABLE_IPV6        
		else if(domain == Socket::IP6) {
			if(inet_pton(af, "::", inaddrp) == 0) {
				Logs::err << "error (pton+ipv6): setBoundAddress:inet_pton:[::]\n";
				exit(EXIT_FAILURE);
			}
		}
#endif //ENABLE_IPV6       
	}
#else //HAVE_INET_PTON
	if(inet_aton(a.c_str(), (struct in_addr *)inaddrp) == 0) {
		if(domain == Socket::IP4) {
			if(inet_aton("0.0.0.0", (struct in_addr *)inaddrp) == 0) {
				Logs::err << "error (aton+ipv4): setBoundAddress:inet_aton:0.0.0.0\n";
				exit(EXIT_FAILURE);
			}
		}
#ifdef ENABLE_IPV6        
		else if(domain == Socket::IP6) {
			if(inet_aton("::", (struct in_addr *)inaddrp) == 0) {
				Logs::err << "error (aton+ipv6): setBoundAddress:inet_aton:[::]\n";
				exit(EXIT_FAILURE);
			}
		}
#endif //ENABLE_IPV6       
	}
#endif //HAVE_INET_PTON

	if(domain == Socket::IP4) {
		((struct sockaddr_in*)saddrp)->sin_port = htons(p);
	}
#ifdef ENABLE_IPV6    
	else if(domain == Socket::IP6) {
		((struct sockaddr_in6*)saddrp)->sin6_port = htons(p);
	}
#endif

	int ret = ::bind(fd, saddrp, saddrl);
	if(ret == -1) {
		Logs::err << "error: bind: " << Util::errnoToString(errno) << endl;
		exit(EXIT_FAILURE);
	}
}

void Socket::accept(int& f, Domain& d) throw()
{
	d = domain;
	f = ::accept(fd, NULL, NULL); // will return -1 on error
}

void Socket::disconnect(const string& msg)
{
	Logs::err << getFd() << " disconnected: " << msg << endl;
	EventManager::instance()->disableRead(getFd());
	disconnected = true;
}

int Socket::read(void* buf, int len) throw(socket_error)
{
	int ret = ::read(getFd(), buf, len);
	if(ret < 0)
		throw socket_error(Util::errnoToString(errno));
	if(ret == 0)
		throw socket_error("normal disconnect");
	return ret;
}

int Socket::write(void* buf, int len) throw(socket_error)
{
	int ret = ::write(getFd(), buf, len);
	if(ret < 0)
		throw socket_error(Util::errnoToString(errno));
	return ret;
}

void Socket::write(string const& s, int prio) throw()
{
	Buffer::Ptr tmp(new Buffer(s, prio));
	writeb(tmp);
}

void Socket::writeb(Buffer::Ptr b) throw()
{
	if(b->size() == 0){
		// no 0-byte sends, please
		return;
	}
#ifdef DEBUG
	Logs::line << getFd() << ">> " << string(b->data(), b->data() + b->size());
#endif
	queue.push(b);
	if(!writeEnabled){
		EventManager::instance()->enableWrite(getFd(), this);
		writeEnabled = true;
	}
}

void Socket::partialWrite()
{
	assert(!queue.empty() && "We got a write-event though we got nothing to write");

	Buffer::Ptr top = queue.front();

	assert(written < (int)top->size() && "We have already written the entirety of this buffer");

	int w = ::write(fd, top->data() + written, top->size() - written);

	if(w < 0){
		switch(errno){
		default:
			while(!queue.empty())
				queue.pop();
			disconnect(Util::toString(w) + ": write failed: " + Util::errnoToString(errno));
			return;
			break;
		}
	} else {
		written += w;

		if(written == (int)top->size()){
			queue.pop();
			written = 0;
		}
	}
}

void Socket::initSocketNames() throw()
{
	if(domain == PF_INET) {
		struct sockaddr_in sa;
		socklen_t n = sizeof(struct sockaddr_in);
		memset(&sa, '\0', n); // necessary?
#ifdef __CYGWIN__
		getsockname(fd, (struct sockaddr*)&sa, &n);
#else        
		assert(getsockname(fd, (struct sockaddr*)&sa, &n) == 0);
#endif
#ifdef HAVE_INET_NTOP
		char buf[INET_ADDRSTRLEN];
		assert(inet_ntop(AF_INET, &sa.sin_addr, buf, INET_ADDRSTRLEN) != 0);
		sockName = buf;
#else
		char *buf2 = inet_ntoa(sa.sin_addr);
		sockName = buf2;
#endif //HAVE_INET_NTOP

		if(getpeername(fd, (struct sockaddr*)&sa, &n) == 0) { // socket may not be connected
#ifdef HAVE_INET_NTOP
			assert(inet_ntop(AF_INET, &sa.sin_addr, buf, INET_ADDRSTRLEN) != 0);
			peerName = buf;
#else
			buf2 = inet_ntoa(sa.sin_addr);
			peerName = buf2;
#endif //HAVE_INET_NTOP
		}

	}
#ifdef ENABLE_IPV6    
	else if(domain == Socket::IP6) {
		struct sockaddr_in6 sa;
		socklen_t n = sizeof(struct sockaddr_in6);
		memset(&sa, '\0', n); // necessary?
		char buf[INET6_ADDRSTRLEN];

		assert(getsockname(fd, (struct sockaddr*)&sa, &n) == 0);
#ifdef HAVE_INET_NTOP
		assert(inet_ntop(AF_INET6, &sa.sin6_addr, buf, INET6_ADDRSTRLEN) != 0);
#else
# error FIXME
#endif
		if(strncmp(buf, "::ffff:", 7) == 0) {
			sockName = buf + 7;
		} else {
			sockName = string("[") + buf + ']';
		}

		if(getpeername(fd, (struct sockaddr*)&sa, &n) == 0) { // socket may not be connected
#ifdef HAVE_INET_NTOP
			assert(inet_ntop(AF_INET6, &sa.sin6_addr, buf, INET6_ADDRSTRLEN) != 0);
#else
# error FIXME
#endif
			if(strncmp(buf, "::ffff:", 7) == 0) {
				ip4OverIp6 = true;
				peerName = buf + 7;
			} else {
				peerName = string("[") + buf + ']';
			}
		}
	}
#endif //ENABLE_IPV6
	else {
		assert(0);
	}
}

bool Socket::setNoLinger() throw()
{
	struct linger so_linger;
	so_linger.l_onoff = 0;
	so_linger.l_linger = 0;
	int ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
	if(ret != 0) {
		Logs::err << "warning: setsockopt:SO_LINGER: " << Util::errnoToString(errno) << endl;
		return false;
	}
	return true;
}

bool Socket::setNonBlocking() throw()
{
	int f;
	if((f = fcntl(fd, F_GETFL, 0)) < 0){
		Logs::err << "warning: fcntl:F_GETFL: " << Util::errnoToString(errno) << endl;
		return false;
	}
	f |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, f) < 0){
		Logs::err << "warning: fcntl:F_SETFL: " << Util::errnoToString(errno) << endl;
		return false;
	}
	return true;
}

bool Socket::setSendTimeout(size_t seconds) throw()
{
	//Set timeout. Various Linuxes will refuse, but not all
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	int ret = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	if(ret != 0) {
		Logs::err << "warning: setsockopt:SO_SNDTIMEO: " << Util::errnoToString(errno) << endl;
		return false;
	}
	return true;
}
