#ifndef __INTERHUB_H_
#define __INTERHUB_H_

#include "Socket.h"

namespace qhub {

class InterHub : public Socket {
public:
	InterHub(int fd);

	void on_read();
	void on_write();
protected:
	int state;
	
	enum State {
		NOT_STARTED,
		PARTIAL_LENGTH,
		PARTIAL_DATA,
		PACKET_READY
	};

	//current offset in packet = rbCur, we always start new packets on the readBuffer boundary
	//current length = first 4 bytes of readBuffer, IF rbCur>3

	void growBuffer();
	unsigned char* readBuffer;
	int readBufferSize;
	int rbCur;
};

}


#endif
