#ifndef _qhub_h_
#define _qhub_h_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event.h>
#include "Socket.h"

namespace qhub {

typedef struct connectionEvent {
	struct event* ev;
	Socket *socket;
} cEvent;

}
#endif
