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

#include "Socket.h"


extern "C" {
#include <oop.h>
#include <adns.h>
}

#define PROTO_DEBUG

namespace qhub {

class DNSUser;

void enable_fd(int fd, oop_event ev, Socket* s);
void cancel_fd(int fd, oop_event ev);

void lookup(const char* hostname, DNSUser* const d);

}
#endif
