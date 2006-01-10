// vim:ts=4:sw=4:noet

#include "DNSAdapter.h"
#include "Logs.h"

using namespace qhub;

#include <sys/types.h>

#ifdef WIN32
#else
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include <ares.h>
}

static void callback(void *arg, int status, struct hostent *host);

DNSAdapter::DNSAdapter(string& hostname)
{
	int status = ares_init(&channel);
	
    if (status != ARES_SUCCESS)
	{
		Logs::stat << "ares_init: " << ares_strerror(status) << endl;
	}

	struct in_addr* addr = new struct in_addr;
	
	addr->s_addr = inet_addr(hostname.c_str());
	//XXX: looking ip numeric ips -> hostname seems to corrupt memory
	if (addr->s_addr == INADDR_NONE) {
		ares_gethostbyname(channel, hostname.c_str(), AF_INET, callback, this);
	} else {
	    ares_gethostbyaddr(channel, addr, sizeof(&addr), AF_INET, callback,	this);
	}

	doHack();
}

DNSAdapter::~DNSAdapter() throw()
{
	Logs::stat << "TIMEOUT!" << endl;
	ares_destroy(channel);
}

bool DNSAdapter::doHack()
{
	fd_set read_fds, write_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	int nfds = ares_fds(channel, &read_fds, &write_fds);
	
	assert(nfds<FD_SETSIZE && "FD_SETSIZE is too small for us.");
	
	if(nfds != 0){
		fd = nfds-1;
		int flag = 0;
		if(FD_ISSET(fd, &read_fds)){
			flag |= ev_read;
		}
		if(FD_ISSET(fd, &write_fds)){
			flag |= ev_write;
		}
		timeval tmp;
		tmp.tv_sec = 5;
		tmp.tv_usec = 0;
		enableMe((type)flag, &tmp);
	}

	return true;
}

void DNSAdapter::onTimeout() throw()
{
	delete this;
}

bool DNSAdapter::onRead() throw()
{
	fd_set read_fds, write_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_SET(fd, &read_fds);

	ares_process(channel, &read_fds, &write_fds);
	
	return doHack();
}

void DNSAdapter::onWrite() throw()
{
	fd_set read_fds, write_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_SET(fd, &write_fds);

	ares_process(channel, &read_fds, &write_fds);
	
	doHack();
}


void DNSAdapter::init()
{
}

static void callback(void *arg, int status, struct hostent *host)
{
	DNSAdapter *us = (DNSAdapter*) arg;
	
	struct in_addr addr;
	char **p;

	if (status != ARES_SUCCESS) {
		Logs::err << "Error: " << ares_strerror(status) << endl;
		return;
	}

	for (p = host->h_addr_list; *p; p++) {
		memcpy(&addr, *p, sizeof(struct in_addr));
		char *textual = inet_ntoa(addr);
		//printf("%-32s\t%s\n", host->h_name, textual);
		if(textual) {
			us->complete(inet_ntoa(addr));
		} else {
			us->complete("");
		}
	}
}
