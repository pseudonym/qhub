// vim:ts=4:sw=4:noet
#include "Socket.h"
#include "qhub.h"

#include "config.h"

using namespace std;
using namespace qhub;

Socket::Socket(Domain d, int t, int p) throw()
: domain(d), ip4OverIp6(false), writeEnabled(false), written(0), disconnected(false)
{
	create();

	fd = ::socket(d, t, p);
	if(fd == -1){
		fprintf(stderr, "Error, could not allocate socket.\n");
		return;
	}

	setNonBlocking();
	setSendTimeout(2);
	initSocketNames();
}

Socket::Socket(int f, Domain d) throw()
: fd(f), domain(d), ip4OverIp6(false), writeEnabled(false), written(0), disconnected(false)
{
	create();
	initSocketNames();
}

Socket::~Socket() throw()
{
	destroy();
}

void Socket::create() throw()
{
	if(domain == PF_INET) {
		saddrp = (struct sockaddr*)new sockaddr_in;
		saddrl = sizeof(sockaddr_in);
		inaddrp = &((struct sockaddr_in*)saddrp)->sin_addr;
		af = AF_INET;
		((struct sockaddr_in*)saddrp)->sin_family = af;
	} else if(domain == PF_INET6) {
		saddrp = (struct sockaddr*)new sockaddr_in6;
		saddrl = sizeof(sockaddr_in6);
		inaddrp = &((struct sockaddr_in6*)saddrp)->sin6_addr;
		af = AF_INET6;
		((struct sockaddr_in6*)saddrp)->sin6_family = af;
	} else {
		assert(0);
	}
	memset(saddrp, '\0', saddrl);
}

void Socket::destroy() throw()
{
	if(domain == PF_INET) {
		delete (struct sockaddr_in*)saddrp;
	} else if(domain == PF_INET6) {
		delete (struct sockaddr_in6*)saddrp;
	}
}

void Socket::setPort(int p) throw()
{
	if(domain == PF_INET) {
		((struct sockaddr_in*)saddrp)->sin_port = htons(p);
	} else if(domain == PF_INET6) {
		((struct sockaddr_in6*)saddrp)->sin6_port = htons(p);
	}	
}

void Socket::setBindAddress(string const& a) throw()
{
#ifdef HAVE_INET_PTON
	if(inet_pton(af, a.c_str(), inaddrp) == 0) {
		if(domain == PF_INET) {
			if(inet_pton(af, "0.0.0.0", inaddrp) == 0) {
				perror("error: setBoundAddress:inet_pton:0.0.0.0");
				exit(1);
			}
		} else if(domain == PF_INET6) {
			if(inet_pton(af, "::", inaddrp) == 0) {
				perror("error: setBoundAddress:inet_pton:[::]");
				exit(1);
			}
		}
	}
#else
# error FIXME
#endif
}

void Socket::listen(int backlog) throw()
{
	int s = ::listen(fd, backlog);
	if(s == 0){
		//printf("Listening in non-blocking mode on port %d with backlog %d.\n", ntohs(saddr_in.sin_port), backlog);
	}
}

void Socket::bind() throw()
{
	int ret = ::bind(fd, saddrp, saddrl);
	if(ret == -1) {
		perror("error: bind");
		exit(1);
	}
}

void Socket::accept(int& f, Domain& d) throw()
{
	d = domain;
	f = ::accept(fd, NULL,  NULL); // will return -1 on error
}

void Socket::disconnect()
{
	cancel_fd(fd, OOP_READ); // stop reading immediately
	disconnected = true;
}

void Socket::write(string const& s, int prio)
{
	Buffer::writeBuffer tmp(new Buffer(s, prio));
	writeb(tmp);
}

void Socket::writeb(Buffer::writeBuffer b)
{
	if(b->getBuf().size() == 0){
		// no 0-byte sends, please
		return;
	}
	queue.push(b);
	if(!writeEnabled){
		enable_fd(fd, OOP_WRITE, this);
		writeEnabled = true;
	}
}

void Socket::partialWrite()
{
	//only try writing _once_, think it helps fairness
	assert(!queue.empty() && "We got a write-event though we got nothing to write");

	Buffer::writeBuffer top = queue.front();

	assert(written < (int)top->getBuf().size() && "We have already written the entirety of this buffer");

	const char* d = top->getBuf().c_str();

	d += written;

	int w = ::write(fd, d, top->getBuf().size()-written);

	if(w < 0){
		switch(errno){
		default:
			while(!queue.empty())
				queue.pop();
			disconnect();
			return;
			break;
		}
	} else {
		written += w;

		if(written == (int)top->getBuf().size()){
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
		char buf[INET_ADDRSTRLEN];
	
		assert(getsockname(fd, (struct sockaddr*)&sa, &n) == 0);
#ifdef HAVE_INET_NTOP
		assert(inet_ntop(AF_INET, &sa.sin_addr, buf, INET_ADDRSTRLEN) != 0);
#else
# error FIXME
#endif
		sockName = buf;
		
		if(getpeername(fd, (struct sockaddr*)&sa, &n) == 0) { // socket may not be connected
#ifdef HAVE_INET_NTOP
			assert(inet_ntop(AF_INET, &sa.sin_addr, buf, INET_ADDRSTRLEN) != 0);
#else
# error FIXME
#endif
			peerName = buf;
		}
		
	} else if(domain == PF_INET6) {
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
		
	} else {
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
		perror("warning: setsockopt:SO_LINGER");
		return false;
	}
	return true;
}

bool Socket::setNonBlocking() throw()
{
	int f;
	if((f = fcntl(fd, F_GETFL, 0)) < 0){
		perror("warning: fcntl:F_GETFL");
		return false;
	}
	f |= O_NONBLOCK;
	if(fcntl(fd, F_SETFL, f) < 0){
		perror("warning: fcntl:F_SETFL");
		return false;
	}
	return true;
}

bool Socket::setSendTimeout(size_t seconds) throw()
{
	//Set timeout. Various Linux:es will refuse, but not all
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	int ret = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	if(ret != 0) {
		perror("warning: setsockopt:SO_SNDTIMEO");
		return false;
	}
	return true;
}
