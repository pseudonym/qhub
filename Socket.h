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
#define __USE_GNU
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace qhub {

class Socket {
public:
	Socket(int d=AF_INET, int t = SOCK_STREAM, int p = 0);

	int socket(void) {return fd;}
	int accept();

	virtual bool on_read() = 0;
	virtual bool on_write() = 0;
protected:

private:
	int fd;

	struct sockaddr_in saddr_in;
};

}


#endif
