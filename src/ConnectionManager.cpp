#include "config.h"

#include "ConnectionManager.h"

#include "InterHub.h"
#include "Client.h"
#include "ADCSocket.h"
#include "ServerSocket.h"
#include "Settings.h"
#include "Logs.h"

#include <boost/lambda/construct.hpp> // for delete_ptr

using namespace qhub;
using namespace std;

ConnectionManager::ConnectionManager() throw()
{
	XmlTok* p = Settings::instance()->getConfig("__connections");
	XmlTok* pp;

	// we only want to do this the first time, not on reloads
	p->findChild("interconnect");
	while((pp = p->getNextChild())) {
		const string& host = pp->getAttr("host");
		int port = Util::toInt(pp->getAttr("port"));
		const string& pass = pp->getAttr("password");
		if(host.empty() || port <= 0 || port > 65535)
			continue;
		Logs::stat << "Connecting to " << host << ':' << port << endl; 
		openInterConnection(host, port, pass);
	}
	load();
}

void ConnectionManager::load() throw()
{
	for_each(listenSocks.begin(), listenSocks.end(), boost::lambda::delete_ptr());
	listenSocks.clear();

	XmlTok* p = Settings::instance()->getConfig("__connections");
	XmlTok* pp;

	p->findChild("clientport");
	while((pp = p->getNextChild())) {
		int port = Util::toInt(pp->getData());
		Logs::stat << "Client Port: " << port << '\n';
		if(port > 0 && port <= 65535)
			openClientPort(port);
	}

	p->findChild("interport");
	while((pp = p->getNextChild())) {
		int port = Util::toInt(pp->getData());
		Logs::stat << "Inter-hub Port: " << port << '\n';
		if(port > 0 && port <= 65535)
			openInterPort(port);
	}
}

void ConnectionManager::openClientPort(int port)
{
	//Leaf-handler
	ServerSocket* tmp = NULL;
#ifdef ENABLE_IPV6
	try {
		tmp = new ServerSocket(Socket::IP6, port, ServerSocket::LEAF_HANDLER);
		listenSocks.push_back(tmp);
		return;
	} catch(const socket_error&) {
		delete tmp;
		// oh well, just try using IPv4...
	}
#endif

	try {
		tmp = new ServerSocket(Socket::IP4, port, ServerSocket::LEAF_HANDLER);
		listenSocks.push_back(tmp);
		return;
	} catch(const socket_error& e) {
		delete tmp;
		Logs::err << "opening client port " << port << "failed: " << e.what() << endl;
	}
}

void ConnectionManager::openInterPort(int port)
{
	//Inter-hub
	ServerSocket* tmp = NULL;
#ifdef ENABLE_IPV6
	try {
		tmp = new ServerSocket(Socket::IP6, port, ServerSocket::INTER_HUB);
		listenSocks.push_back(tmp);
		return;
	} catch(const socket_error&) {
		delete tmp;
	}
#endif

	try {
		tmp = new ServerSocket(Socket::IP4, port, ServerSocket::INTER_HUB);
		listenSocks.push_back(tmp);
		return;
	} catch(const socket_error& e) {
		delete tmp;
		Logs::err << "opening server port " << port << "failed: " << e.what() << endl;
	}
}

void ConnectionManager::acceptLeaf(int fd, Socket::Domain d)
{
	// looks odd, but does what it's supposed to
	new Client(new ADCSocket(fd, d));
}

void ConnectionManager::openInterConnection(const string& host, int port, const string& pass) throw()
{
	//we don't want this added anywhere until it's functional
	//it will add itself once it's ready to carry traffic
	new InterHub(host, (short)port, pass);
}

void ConnectionManager::acceptInterHub(int fd, Socket::Domain d)
{
	//see comment above
	new InterHub(new ADCSocket(fd, d));
}

