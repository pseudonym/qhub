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

//#include "Client.h"
#include "ServerSocket.h"
#include "InterHub.h"
#include "ADC.h"
//#include "ClientPool.h"

using namespace qhub;

ServerSocket::ServerSocket(int port, int t) : Socket(AF_INET), type(t) {
    int yes=1;

	if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		printf("Error setting reuse address.\n");
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
	set_bound_address(0);
	bind();
	listen();
}

void ServerSocket::on_read(){
	while(true){
		int fd = Socket::accept();
		if(fd != -1){
			fprintf(stderr, "Accepted socket %d\n", fd);
			Socket* tmp;
			switch(type){
				case INTER_HUB:
					tmp = new InterHub(fd);
					break;
				case LEAF_HANDLER:
					tmp = new ADC(fd);
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
}

void ServerSocket::on_write(){
	printf("Serversocket received a write.\n");
	exit(1);
}

