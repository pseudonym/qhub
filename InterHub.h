#ifndef __INTERHUB_H_
#define __INTERHUB_H_

#include "Socket.h"

#include <string>

using namespace std;


namespace qhub {

class Hub;

class InterHub : public Socket {
public:
	InterHub(Hub* h) { hub = h; };
	InterHub(int fd);

	void on_read();
	void on_write();

	void setHostName(string h) { hostname = h; }
	void setPassword(string p) { password = p; }
	void setPort(int p) { port = p; }
	int getPort() { return port; }

	void connect();
protected:
	Hub* hub;
	int state;
	
	void sendData(string data);
	void sendDData(string data, string dest);

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

	string hostname;
	string password;
	int port;

	void handlePacket();

	void realDisconnect();
};

}


#endif
