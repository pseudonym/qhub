#include "Socket.h"

using namespace std;

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
		fprintf(stderr, "Error, couldnt fcntl().\n");
	}

	f |= O_NONBLOCK;

	if (fcntl(fd, F_SETFL, f) < 0){
		//XXX do more, but what?
		fprintf(stderr, "Error, couldnt fcntl().\n");
	}

	memset(&saddr_in, '\0', sizeof(sockaddr_in));

	saddr_in.sin_family = d;
}

void Socket::setPort(int p)
{
	saddr_in.sin_port = htons(p);
}

void Socket::set_bound_address(int a)
{
	saddr_in.sin_addr.s_addr = htonl(a);
}

void Socket::listen(int backlog)
{
	int s = ::listen(fd, backlog);
	if(s == 0){
		//printf("Listening in non-blocking mode on port %d with backlog %d.\n", ntohs(saddr_in.sin_port), backlog);
	}
}

void Socket::bind()
{
	int ret;

	ret = ::bind(fd, (struct sockaddr *)&saddr_in, sizeof(saddr_in));

	if (ret == -1){
		printf("Error binding.\n");
		exit(1);
	}
}

int Socket::accept()
{
	//will return -1 on error
	return ::accept(fd, NULL,  NULL);
}

}

