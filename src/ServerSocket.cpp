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
#include "Logs.h"
#include "ServerSocket.h"
#include "InterHub.h"
#include "Hub.h"
#include "ConnectionManager.h"

using namespace std;
using namespace qhub;

ServerSocket::ServerSocket(Domain domain, int port, int t) : Socket(domain), type(t) {
	int yes = 1;

	if(setsockopt(getFd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		Logs::err << "warning: setsockopt:SO_REUSEADDR: " << Util::errnoToString(errno) << endl;
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

	bind(Util::emptyString, port);	// empty = use INADDR_ANY
	listen();

	EventManager::instance()->enableRead(getFd(), this);
}

ServerSocket::~ServerSocket() throw()
{
	if(getFd() != -1)
		close(getFd());
}

void ServerSocket::onRead(int) throw()
{
	while(true){
		int fd;
		Domain d;
		Socket::accept(fd, d);
		if(fd != -1){
			switch(type){
			case INTER_HUB:
				Logs::stat << "accepted ihub socket " << fd << endl;
				ConnectionManager::instance()->acceptInterHub(fd, d);
				break;
			case LEAF_HANDLER:
				Logs::stat << "accepted leaf socket " << fd << endl;
				ConnectionManager::instance()->acceptLeaf(fd, d);
				break;
			default:
				assert(0 && "unknown type for listening socket.");
			}
		} else {
			break;
		}
	}
}

void ServerSocket::onWrite(int) throw()
{
	assert(0 && "ServerSocket received a write.");
}

