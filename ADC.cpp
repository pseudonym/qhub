#include "ADC.h"
#include "qhub.h"


#define START_BUFFER 1024
#define READ_SIZE 512

using namespace qhub;

ADC::ADC(int fd) : state(START), readBuffer(new unsigned char[START_BUFFER]), readBufferSize(START_BUFFER), rbCur(0)
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

void ADC::growBuffer()
{
	unsigned char* tmp = new unsigned char[readBufferSize*2];
	memmove(tmp, readBuffer, readBufferSize);
	readBufferSize *= 2;

	delete[] readBuffer;
	readBuffer = tmp;

	fprintf(stderr, "Growing buffer to %d\n", readBufferSize);
}

void ADC::handleCommand(int length)
{
	//there is a command in readBuffer, starting at index 0.
	fprintf(stderr, "Command: %d ", length);
	for(int i=0; i<length-1; i++){
		fprintf(stderr, "%d '%c'", readBuffer[i], readBuffer[i]);
	}
	fprintf(stderr, "\n");

	//now, this command is handled. Remove it.
	memmove(readBuffer, readBuffer+length, readBufferSize-length);
	rbCur -= length;
}

void ADC::on_read()
{
	while(rbCur+READ_SIZE >= readBufferSize){
		growBuffer();
	}
	int r = read(fd, readBuffer+rbCur, READ_SIZE);
	if(r > 0){
		fprintf(stderr, "Got data %d %d %d\n", rbCur, r, readBufferSize);
		rbCur += r;

		//look through data until no more left?
		while(rbCur>0){
			for(int i=0; i<rbCur; i++){
				if(readBuffer[i] == 0x0a){
					if(i>0){
						if(readBuffer[i-1] != '\\'){
							//not escaped
							handleCommand(i+1);
							break;
						}
					} else {
						//not escaped
						handleCommand(i+1);
						break;
					}
				}
			}
			//rbCur -> 0 on handleCommand
			if(rbCur != 0){
				break;
			}
		}
	} else if(r < 1){
		cancel(fd, OOP_READ);
		close(fd);
		delete[] readBuffer;
		//XXX someone (other than us?) also needs to deallocate us
		delete this;
	}
}

void ADC::on_write()
{

}
