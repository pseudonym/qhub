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

using namespace std;

#define PRIO_NORM 200
#define PRIO_EINF 300
#define PRIO_LIST 400
//Need another priority for quits.
#define PRIO_QUIT 350

namespace qhub {

class Socket {
public:
	Socket(int d=AF_INET, int t = SOCK_STREAM, int p = 0);

	int accept();

	virtual void on_read() = 0;
	virtual void on_write() = 0;

	void setPort(int p);
	void set_bound_address(int a);
	void bind();
	void listen(int backlog = 8192);

	//beware: this will copy string. Limit use.
	void write(string& s, int prio=PRIO_NORM);
	void w(Buffer::writeBuffer b);

	int getFd() { return fd; };
	string getSockName() const;
	string getPeerName() const;
protected:
	int fd;
	struct sockaddr_in saddr_in;

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
};

}


#endif
