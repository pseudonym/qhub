#ifndef QHUB_SOCKET_H
#define QHUB_SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <cerrno>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <queue>

#include "config.h"
#include "Buffer.h"
#include "Util.h"
#include "EventManager.h"


// these should probably be deprecated, as they aren't actually used
// (we're just using normal queues for actual output)
#define PRIO_NORM 200
#define PRIO_EINF 300
#define PRIO_LIST 400
//Need another priority for quits.
#define PRIO_QUIT 350

namespace qhub {

class Socket : public EventListener {
public:
	enum Domain {
		IP4 = PF_INET, 
#ifdef ENABLE_IPV6
		IP6 = PF_INET6 
#endif
	};

	Socket(Domain d = IP4, int t = SOCK_STREAM, int p = 0) throw(socket_error); // new sockets
	Socket(int fd, Domain d) throw(); // existing sockets
	virtual ~Socket() throw();

	// socket options
	bool setNoLinger() throw();
	bool setNonBlocking() throw();
	bool setSendTimeout(size_t seconds) throw();

	void connect(const std::string& ip, short port) throw(socket_error);
	void bind(const std::string& a, short port) throw();
	void listen(int backlog = 8192) throw(socket_error);
	void accept(int& fd, Domain& d) throw();

	int read(void* buf, int len) throw(socket_error);
	int write(void* buf, int len) throw(socket_error);
	//beware: this will copy string. Limit use.
	void write(std::string const& s, int prio = PRIO_NORM) throw();
	void writeb(Buffer::Ptr b) throw();

	int getFd() const throw() { return fd; }
	Domain getDomain() const throw() { return ip4OverIp6 ? IP4 : domain; };
	std::string const& getSockName() const throw() { return sockName; };
	std::string const& getPeerName() const throw() { return peerName; };

protected:
	int fd;
	Domain domain;
	int af;
	struct sockaddr* saddrp;
	socklen_t saddrl;
	void* inaddrp;

	std::string sockName, peerName;
	bool ip4OverIp6;
	
	//output queue
	//priority_queue<Buffer::Ptr> queue;
	std::queue<Buffer::Ptr> queue;

	void partialWrite();
	bool writeEnabled;
	//how much written for topmost Buffer
	int written;

	//signals that we should die
	bool disconnected;
	virtual void disconnect(const std::string& msg = Util::emptyString);

private:
	void create() throw();
	void destroy() throw();
	void initSocketNames() throw();
};

} // namespace qhub

#endif // QHUB_SOCKET_H
