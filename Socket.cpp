// vim:ts=4:sw=4:noet
#include "Socket.h"
#include "qhub.h"

using namespace std;
using namespace qhub;

Socket::Socket(int d, int t, int p) : written(0), writeEnabled(false), disconnected(false)
{
	fd = ::socket(d, t, p);


	if(fd == -1){
		fprintf(stderr, "Error, could not allocate socket.\n");
		return;
	}

	int f;
	//Set nonblocking
	if ( (f = fcntl(fd, F_GETFL, 0)) < 0){
		fprintf(stderr, "Error, couldnt fcntl().\n");
	}

	f |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, f) < 0){
		fprintf(stderr, "Error, couldnt fcntl().\n");
	}

	//Set timeout. Various Linux:es will refuse, but not all
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	int ret = setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	if(ret != 0) {
		fprintf(stderr, "Error setting SO_SNDTIMEO\n");
	}

	memset(&saddr_in, '\0', sizeof(sockaddr_in));

	saddr_in.sin_family = d;
}

void Socket::setPort(int p)
{
	saddr_in.sin_port = htons(p);
}

void Socket::set_bound_address(int a)
{
	saddr_in.sin_addr.s_addr = htonl(a);
}

void Socket::listen(int backlog)
{
	int s = ::listen(fd, backlog);
	if(s == 0){
		//printf("Listening in non-blocking mode on port %d with backlog %d.\n", ntohs(saddr_in.sin_port), backlog);
	}
}

void Socket::bind()
{
	int ret;

	ret = ::bind(fd, (struct sockaddr *)&saddr_in, sizeof(saddr_in));

	if (ret == -1){
		printf("Error binding.\n");
		exit(1);
	}
}

int Socket::accept()
{
	//will return -1 on error
	return ::accept(fd, NULL,  NULL);
}

void Socket::disconnect()
{
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
		//no 0-byte sends, please
		return;
	}
	queue.push(b);
	if(!writeEnabled){
		enable(fd, OOP_WRITE, this);
		writeEnabled=true;
	}
}

void Socket::partialWrite()
{
	//only try writing _once_, think it helps fairness
	assert(!queue.empty() && "We got a write-event though we got nothing to write");

	Buffer::writeBuffer top = queue.front();

	assert(written<top->getBuf().size() && "We have already written the entirety of this buffer");

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

		if(written == top->getBuf().size()){
			queue.pop();
			written = 0;
		}
	}
}

string Socket::getSockName() const {
	struct sockaddr_in sa;
	socklen_t n = sizeof(struct sockaddr_in);
	if(getsockname(fd, (struct sockaddr*)&sa, &n) == 0) {
		char buf[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &sa.sin_addr, buf, INET_ADDRSTRLEN);
		return buf;
	} else {
		return "(error)";
	}
}

string Socket::getPeerName() const {
	struct sockaddr_in sa;
	socklen_t n = sizeof(struct sockaddr_in);
	if(getpeername(fd, (struct sockaddr*)&sa, &n) == 0) {
		char buf[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &sa.sin_addr, buf, INET_ADDRSTRLEN);
		return buf;
	} else {
		return "(error)";
	}
}

bool Socket::setNoLinger() {
	struct linger so_linger;
	// Set linger to false
	so_linger.l_onoff = 0;
	so_linger.l_linger = 0;
	int ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
	if(ret != 0) {
		fprintf(stderr, "Error setting SO_LINGER\n");
		return false;
	}
	return true;
}
