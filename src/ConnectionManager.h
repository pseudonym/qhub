#ifndef QHUB_CONNECTIONMANAGER_H
#define QHUB_CONNECTIONMANAGER_H

#include "qhub.h"
#include "Singleton.h"
#include "Socket.h"

#include <string>
#include <vector>

namespace qhub {

class ConnectionManager : public Singleton<ConnectionManager> {
public:
	void openClientPort(int port);
	void openInterPort(int port);
	void openInterConnection(const std::string&, int port, const std::string&) throw();

	void acceptLeaf(int fd, Socket::Domain d);
	void acceptInterHub(int fd, Socket::Domain d);

	void load() throw();

private:
	friend class Singleton<ConnectionManager>;

	typedef std::vector<ServerSocket*> SockList;

	SockList listenSocks;

	ConnectionManager() throw();
	~ConnectionManager() throw() {}
};

} // namespace qhub

#endif // QHUB_CONNECTIONMANAGER_H
