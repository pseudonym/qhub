#ifndef _SOCKET_H_
#define _SOCKET_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <queue>
#include <boost/shared_ptr.hpp>

#include "Buffer.h"
#include "Util.h"

using namespace std;

#define PRIO_NORM 200
#define PRIO_EINF 300
#define PRIO_LIST 400
//Need another priority for quits.
#define PRIO_QUIT 350

namespace qhub {

class Socket {
public:
	enum Domain { IP4 = PF_INET, IP6 = PF_INET6 };
	Socket(Domain d = IP4, int t = SOCK_STREAM, int p = 0) throw(); // new sockets
	Socket(int fd, Domain d) throw(); // existing sockets
	virtual ~Socket() throw();

	virtual void on_read() = 0;
	virtual void on_write() = 0;

	// socket options
	bool setNoLinger() throw();
	bool setNonBlocking() throw();
	bool setSendTimeout(size_t seconds) throw();

	void setPort(int p) throw();
	void setBindAddress(string const& a = Util::emptyString) throw();
	void bind() throw();
	void listen(int backlog = 8192) throw();
	void accept(int& fd, Domain& d) throw();

	//beware: this will copy string. Limit use.
	void write(string const& s, int prio = PRIO_NORM);
	void writeb(Buffer::writeBuffer b);

	int getFd() { return fd; };
	Domain getDomain() const throw() { return ip4OverIp6 ? IP4 : domain; };
	string const& getSockName() const throw() { return sockName; };
	string const& getPeerName() const throw() { return peerName; };

protected:
	int fd;
	Domain domain;
	int af;
	struct sockaddr* saddrp;
	socklen_t saddrl;
	void* inaddrp;

	string sockName, peerName;
	bool ip4OverIp6;

	//output queue
	//priority_queue<Buffer::writeBuffer> queue;
	queue<Buffer::writeBuffer> queue;

	void partialWrite();
	bool writeEnabled;
	//how much written for topmost Buffer
	int written;

	//signals that we should die
	bool disconnected;
	void disconnect();

private:
	void create() throw();
	void destroy() throw();
	void initSocketNames() throw();
};

}


#endif
