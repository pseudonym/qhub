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

#include "error.h"
#include "ServerSocket.h"
#include "InterHub.h"
#include "Hub.h"

using namespace qhub;

ServerSocket::ServerSocket(Domain domain, int port, int t, Hub* h) : Socket(domain), type(t), hub(h) {
	int yes = 1;

	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		log(qerr, format("warning: setsockopt:SO_REUSEADDR: %s") % Util::errnoToString(errno));
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

void ServerSocket::onRead() throw()
{
	while(true){
		int fd;
		Domain d;
		Socket::accept(fd, d);
		if(fd != -1){
			switch(type){
			case INTER_HUB:
				log(qstat, format("accepted ihub socket %d") % fd);
				hub->acceptInterHub(fd, d);
				break;
			case LEAF_HANDLER:
				log(qstat, format("accepted leaf socket %d") % fd);
				hub->acceptLeaf(fd, d);
				break;
			default:
				assert(0 && "unknown type for listening socket.");
			}
		} else {
			break;
		}
	}
}

void ServerSocket::onWrite() throw()
{
	assert(0 && "ServerSocket received a write.");
}

