#include "InterHub.h"

#include "qhub.h"

#include <netinet/in.h>

#define START_BUFFER 1024
#define READ_SIZE 1024

using namespace qhub;

InterHub::InterHub(int fd) : state(NO_DATA), readBuffer(new unsigned char[START_BUFFER]), readBufferSize(START_BUFFER), rbCur(0)
{
	this->fd = fd;
	struct linger       so_linger;
    // Set linger to false
    so_linger.l_onoff = false;
    so_linger.l_linger = 0;
    int itmp = setsockopt(fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
	if(itmp != 0){
		fprintf(stderr, "Error setting SO_LINGER\n");
	}

	enable(fd, OOP_READ, this);
}

void InterHub::connect()
{

}

void InterHub::sendDData(string data, string dest)
{

}

void InterHub::sendData(string data)
{
	//wrap in normal header, without destination-field
}

void InterHub::growBuffer()
{
	unsigned char* tmp = new unsigned char[readBufferSize*2];
	memmove(tmp, readBuffer, readBufferSize);
	readBufferSize *= 2;

	delete[] readBuffer;
	readBuffer = tmp;

	fprintf(stderr, "Growing buffer to %d\n", readBufferSize);
}

void InterHub::on_read()
{
	if(rbCur+READ_SIZE >= readBufferSize){
		growBuffer();
	}
	int r = read(fd, readBuffer+rbCur, READ_SIZE);
	if(r > 0){
		fprintf(stderr, "Got data\n");
		rbCur += r;
		if(rbCur>3){
			unsigned int l = *((int*)readBuffer);
			l = htonl(l);
			fprintf(stderr, "Got length %d\n", l);
		}
	} else if(r < 1){
		cancel(fd, OOP_READ);
		close(fd);
		delete[] readBuffer;
		//XXX someone (other than us?) also needs to deallocate us
		delete this;
	}
}

void InterHub::on_write()
{
}
