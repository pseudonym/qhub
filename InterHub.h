#ifndef __INTERHUB_H_
#define __INTERHUB_H_

#include "Socket.h"

namespace qhub {

class InterHub : public Socket {
public:
	InterHub(int fd);

	void on_read();
	void on_write();
};

}


#endif
