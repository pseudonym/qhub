// vim:ts=4:sw=4:noet
#include "Settings.h"
#include "Hub.h"
#include "XmlTok.h"

using namespace qhub;
using namespace std;

int Settings::readFromXML() throw()
{
	XmlTok root;
	if(!root.load("Settings.xml"))
		return 1;

	XmlTok* p = &root;
	if(p->findChild("config")) {
		p = p->getNextChild();
		
		// list all hubs
		XmlTok* hubp;
		p->findChild("hub");
		while((hubp = p->getNextChild())) {
			Hub* hub = new Hub();
			
			if(hubp->findChild("name")) {
				string name = hubp->getNextChild()->getData();
				fprintf(stderr, "Hubname: %s\n", name.c_str());
				hub->setHubName(name);
			}
			if(hubp->findChild("port")) {
				int port = Util::toInt(hubp->getNextChild()->getData());
				fprintf(stderr, "\tADC Port: %i\n", port);
				if(port > 0 && port <= 65535)
					hub->openADCPort(port);
			}	
			if(hubp->findChild("maxpacketsize")) {
				int size = Util::toInt(hubp->getNextChild()->getData());
				fprintf(stderr, "\tMax Packet Size: %i\n", size);
				if(size > 0)
					hub->setMaxPacketSize(size);
			}
			if(hubp->findChild("interport")) {
				int port = Util::toInt(hubp->getNextChild()->getData());
				fprintf(stderr, "\tInter-hub Port: %i\n", port);
				if(port > 0 && port <= 65535)
					hub->openInterPort(port);
			}

			XmlTok* ichubp;
			hubp->findChild("interconnect");
			while((ichubp = hubp->getNextChild())) {
				string host, password;
				int port;
				
				if(ichubp->findChild("host"))
					host = ichubp->getNextChild()->getData();
				if(ichubp->findChild("port"))
					port = Util::toInt(ichubp->getNextChild()->getData());
				if(ichubp->findChild("password"))
					password = ichubp->getNextChild()->getData();
					
				fprintf(stderr, "\tConnecting to %s:%i pass:%s\n", host.c_str(), port, password.c_str());
				hub->openInterConnection(host, port, password);
			}
		}
	}
	return 0;
}
			
			
						
