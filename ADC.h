#ifndef __ADC_H_
#define __ADC_H_

#include "Socket.h"

namespace qhub {

class ADC : public Socket {
public:
	ADC(int fd);

	void on_read();
	void on_write();
private:
	ADC(){};

	enum State {
		START,
		PROTOCOL_ERROR //has sent message to client, at next data, disconnect
	};
	int state;
	
	void growBuffer();
	unsigned char* readBuffer;
	int readBufferSize;
	int rbCur;

	void handleCommand(int length);
};


}

#endif
