#include "Socket.h"

namespace qhub {


Socket::Socket(int d, int t, int p)
{
	fd = ::socket(d, t, p);


	if(fd == -1){
		fprintf(stderr, "Error, could not allocate socket.\n");
		return;
	}

	int f;
	//Set nonblocking
	if ( (f = fcntl(fd, F_GETFL, 0)) < 0){
		//XXX do more, but what?
		printf("Error, couldnt fcntl().\n");
	}

	f |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, f) < 0){
		//XXX do more, but what?
		fprintf(stderr, "Error, couldnt fcntl().\n");
	}

	memset(&saddr_in, '\0', sizeof(sockaddr_in));

	saddr_in.sin_family = d;
}

int Socket::accept()
{
	int Fd = ::accept(fd, NULL,  NULL);
	if(Fd == -1){
		printf("Error accepting socket.\n");
		perror("socket");
		exit(1);
	}

	return Fd;
}

}

