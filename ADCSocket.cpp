// vim:ts=4:sw=4:noet
#include "ADCSocket.h"

#define START_BUFFER 1024
#define READ_SIZE 512

using namespace std;
using namespace qhub;

ADCSocket::ADCSocket(int fd)
: readBufferSize(START_BUFFER), readBuffer(new unsigned char[readBufferSize]), state(NORMAL)
{
	this->Socket::fd = fd;
	setNoLinger();
}

ADCSocket::~ADCSocket()
{
	delete[] readBuffer;
}

/*
void ADC::growBuffer()
{
	unsigned char* tmp = new unsigned char[readBufferSize*2];
	memmove(tmp, readBuffer, readBufferSize);
	readBufferSize *= 2;
	delete[] readBuffer;
	readBuffer = tmp;
	fprintf(stderr, "Growing buffer to %d\n", readBufferSize);
}
*/

void ADCSocket::on_read()
{
	// TODO add some read limits!

	int ret = read(fd, readBuffer, readBufferSize);
	if(ret <= 0) {
		fprintf(stderr, "[%i] Got %i from read()\n", fd, ret);
		perror("read");
		// do some disconnect action here..
	} else {
		unsigned char *p = readBuffer;
		unsigned char *e = readBuffer + ret;
		unsigned char *s = p;
		bool escaped = false;
		for(; p != e; ++p) {
			switch(*p) {
			case '\\':
					escaped = !escaped;
					break;
			case '\n':
			case ' ':
				if(!escaped) {
					if(state == NORMAL) {
						data.push_back(string((char const*)s, p - s));
						s = p + 1;
					} else if(state == PARTIAL) {
						data.back().append((char const*)s, p - s);
						state = NORMAL;
						s = p + 1;
					} else {
						assert(0);
					}
					if(*p == '\n') {
						// Call our dear virtual command handler
						onLine(data);
						data.clear();
					}
				} else {
					escaped = false;
				}
			default:
				escaped = false;
			}
		}
		// add leftover data
		if(s != p) {
			data.push_back(string((char const*)s, p - s));
			state = PARTIAL;
		}
	}
}

void ADCSocket::on_write()
{
}
