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

#include "ServerSocket.h"
#include "InterHub.h"
#include "Hub.h"

using namespace qhub;

ServerSocket::ServerSocket(Domain domain, int port, int t, Hub* h) : Socket(domain), type(t), hub(h) {
	int yes = 1;

	if(setsockopt(getSocket(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("warning: setsockopt:SO_REUSEADDR");
	}

	//just let the kernel handle this for us...
	//2.6 is probably pretty good at it.
	/*yes = 16384;
	//yes = 1024;
	if (setsockopt(fd,SOL_SOCKET,SO_SNDBUF,&yes,sizeof(int)) == -1) {
		printf("Error setting send buffer.\n");
	   }
	if (setsockopt(fd,SOL_SOCKET,SO_RCVBUF,&yes,sizeof(int)) == -1) {
		printf("Error setting receive buffer.\n");
	   }*/

	setPort(port);
	setBindAddress(); // empty = use INADDR_ANY
	bind();
	listen();
}

bool ServerSocket::onRead() throw()
{
	while(true){
		int fd;
		Domain d;
		Socket::accept(fd, d);
		if(fd != -1){
			fprintf(stderr, "Accepted socket %d\n", fd);
			Socket* tmp;
			switch(type){
			case INTER_HUB:
				hub->acceptInterHub(fd, d);
				break;
			case LEAF_HANDLER:
				hub->acceptLeaf(fd, d);
				break;
			default:
				fprintf(stderr, "Closing socket: unknown type for listening socket.\n");
				close(fd);
				break;
			}
		} else {
			break;
		}
	}

	return true;
}

void ServerSocket::onWrite() throw()
{
	printf("Serversocket received a write.\n");
	exit(1);
}

