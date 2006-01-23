#ifndef _SOCKET_H_
#define _SOCKET_H_
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
#include "EventHandler.h"

using namespace std;

#define PRIO_NORM 200
#define PRIO_EINF 300
#define PRIO_LIST 400
//Need another priority for quits.
#define PRIO_QUIT 350

namespace qhub {

class Socket : public EventHandler {
public:
	enum Domain { IP4 = PF_INET, 
#ifdef ENABLE_IPV6
        IP6 = PF_INET6 
#endif
        };
	Socket(Domain d = IP4, int t = SOCK_STREAM, int p = 0) throw(socket_error); // new sockets
	Socket(int fd, Domain d) throw(); // existing sockets
	virtual ~Socket() throw();

	virtual bool onRead() throw() = 0;
	virtual void onWrite() throw() = 0;

	// socket options
	bool setNoLinger() throw();
	bool setNonBlocking() throw();
	bool setSendTimeout(size_t seconds) throw();

	void connect(const string& ip, short port) throw(socket_error);
	void bind(const string& a, short port) throw();
	void listen(int backlog = 8192) throw();
	void accept(int& fd, Domain& d) throw();

	//beware: this will copy string. Limit use.
	void write(string const& s, int prio = PRIO_NORM);
	void writeb(Buffer::Ptr b);

	Domain getDomain() const throw() { return ip4OverIp6 ? IP4 : domain; };
	string const& getSockName() const throw() { return sockName; };
	string const& getPeerName() const throw() { return peerName; };

	bool error() { return err; };

protected:
	Domain domain;
	int af;
	struct sockaddr* saddrp;
	socklen_t saddrl;
	void* inaddrp;

	string sockName, peerName;
	bool ip4OverIp6;
	
	bool err;

	//output queue
	//priority_queue<Buffer::writeBuffer> queue;
	queue<Buffer::Ptr> queue;

	void partialWrite();
	bool writeEnabled;
	//how much written for topmost Buffer
	int written;

	//signals that we should die
	bool disconnected;
	virtual void disconnect(const string& msg = Util::emptyString);

private:
	void create() throw();
	void destroy() throw();
	void initSocketNames() throw();
};

}


#endif
