#ifndef _serversocket_h_
#define _serversocket_h_

#include "Socket.h"

namespace qhub {

class ServerSocket : public Socket{
public:
	ServerSocket(int port, int type);

	enum socketTypes {
		INTER_HUB,
		LAST
	};

protected:
	void on_read(); 
	void on_write();
	
	int type;
};

}

#endif
