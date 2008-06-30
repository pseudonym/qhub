#ifndef QHUB_SERVERSOCKET_H
#define QHUB_SERVERSOCKET_H

#include "qhub.h"
#include "Socket.h"

namespace qhub {

class ServerSocket : public Socket {
public:
	enum ListenType {
		INTER_HUB,
		LEAF_HANDLER,
	};

	ServerSocket(Domain domain, uint16_t port, ListenType type);
	~ServerSocket() throw();

	virtual void onRead(int) throw();

protected:
	ListenType type;
};

} // namespace qhub

#endif // QHUB_SERVERSOCKET_H
