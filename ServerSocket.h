#ifndef _serversocket_h_
#define _serversocket_h_

#include "Socket.h"

namespace qhub {

class Hub;

class ServerSocket : public Socket {
public:
	ServerSocket(Domain domain, int port, int type, Hub* h);

	enum socketTypes {
	    INTER_HUB,
	    LEAF_HANDLER,
	    LAST
	};

protected:
	void on_read();
	void on_write();

	int type;
	Hub* hub;
};

}

#endif
