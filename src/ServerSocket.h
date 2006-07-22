#ifndef QHUB_SERVERSOCKET_H
#define QHUB_SERVERSOCKET_H

#include "Socket.h"

namespace qhub {

class Hub;

class ServerSocket : public Socket {
public:
	ServerSocket(Domain domain, int port, int type);

	enum socketTypes {
	    INTER_HUB,
	    LEAF_HANDLER,
	    LAST
	};

protected:
	virtual void onRead(int) throw();
	virtual void onWrite(int) throw();

	int type;
};

}

#endif // QHUB_SERVERSOCKET_H
