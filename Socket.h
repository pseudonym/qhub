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

namespace qhub {

class Socket {
public:
	Socket(int d=AF_INET, int t = SOCK_STREAM, int p = 0);

	int socket(void) {return fd;}
	int accept();

	virtual void on_read() = 0;
	virtual void on_write() = 0;

	void setPort(int p);
	void set_bound_address(int a);
	void bind();
	void listen(int backlog = 8192);

	int getFd() { return fd; };
protected:
	int fd;
	struct sockaddr_in saddr_in;
};

}


#endif
