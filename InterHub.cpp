#include "InterHub.h"

#include "qhub.h"

#include <netinet/in.h>

#define START_BUFFER 1024
#define READ_SIZE 512

using namespace qhub;

InterHub::InterHub(int fd, Domain d) throw()
: Socket(fd, d), state(NO_DATA), readBuffer(new unsigned char[START_BUFFER]), readBufferSize(START_BUFFER), rbCur(0)
{
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
	sendData(password);
}

void InterHub::sendDData(string data, string dest)
{

}

void InterHub::sendData(string data)
{
	//wrap in normal header, without destination-field
	char header[12];
	unsigned int* i = (unsigned int*) &header[0];
	//length
	i[0] = htonl(12 + data.size());
	memcpy(&header[4], "SPAS", 4);
	//our GUID
	i[2] = htonl(54);

	string t(header, 12);
	t += data;
	Socket::write(t);
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

void InterHub::handlePacket()
{
	switch(readBuffer[4]){
	case 'S':
		if(readBuffer[5] == 'P'){
			//check password
			//if(readBuffer[13] == ....
			fprintf(stderr, "Got password from %d\n", readBuffer[8]);
		}
		break;
	}
}

void InterHub::on_read()
{
	fprintf(stderr, "Got data\n");
	if(rbCur+READ_SIZE >= readBufferSize){
		growBuffer();
	}
	int r = read(fd, readBuffer+rbCur, READ_SIZE);
	if(r > 0){
		fprintf(stderr, "Got data\n");
		rbCur += r;
		while(rbCur > 3) {
			unsigned int l = *((int*)readBuffer);
			l = ntohl(l);
			fprintf(stderr, "Got length %d out of %d\n", l, rbCur);
			if(rbCur >= l) {
				fprintf(stderr, "Got all data in packet\n");
				//handle packet
				handlePacket();
				//and remove it
				rbCur -= l;
				memmove(readBuffer, readBuffer+l, readBufferSize-l);
			} else {
				break;
			}
		}
	} else if(r < 1){
		disconnect();
	}

	if(disconnected){
		realDisconnect();
	}
}

void InterHub::realDisconnect()
{
	if(fd == -1){
		//already done
		return;
	}
	fprintf(stderr, "Disconnecting Interhub %d %p\n", fd, this);

	close(fd);
	cancel(fd, OOP_READ);
	if(writeEnabled){
		cancel(fd, OOP_WRITE);
	}

	fd = -1;

	//what about deleting us?
	delete this;
}

void InterHub::on_write()
{
	fprintf(stderr, "On_write\n");

	//in Socket
	partialWrite();

	if(queue.empty()){
		cancel(fd, OOP_WRITE);
		writeEnabled=false;
	}

	if(disconnected){
		realDisconnect();
	}
}
