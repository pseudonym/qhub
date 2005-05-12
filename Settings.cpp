// vim:ts=4:sw=4:noet
#include <cstdio>
#include <cassert>

#include "error.h"

#include "Settings.h"
#include "Hub.h"
#include "XmlTok.h"
#include "Util.h"

using namespace qhub;
using namespace std;

int Settings::readFromXML() throw()
{
	XmlTok root;
	if(!root.load(CONFIGDIR "/Settings.xml"))
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
				log(qstat, "Hubname: " + name);
				hub->setHubName(name);
			}
			if(hubp->findChild("cid")) {
				string cid = hubp->getNextChild()->getData();
				log(qstat, "\tHub CID: " + cid);
				hub->setCID32(cid);
			}
			if(hubp->findChild("description")) {
				string desc = hubp->getNextChild()->getData();
				log(qstat, "\tHub description: " + desc);
				hub->setDescription(desc);
			}
			if(hubp->findChild("port")) {
				int port = Util::toInt(hubp->getNextChild()->getData());
				log(qstat, "\tADC Port: " + Util::toString(port));
				if(port > 0 && port <= 65535)
					hub->openADCPort(port);
			}
			if(hubp->findChild("maxpacketsize")) {
				int size = Util::toInt(hubp->getNextChild()->getData());
				log(qstat, "\tMax Packet Size: " + Util::toString(size));
				if(size > 0)
					hub->setMaxPacketSize(size);
			}
			if(hubp->findChild("interport")) {
				int port = Util::toInt(hubp->getNextChild()->getData());
				log(qstat, "\tInter-hub Port: " + Util::toString(port));
				if(port > 0 && port <= 65535)
					hub->openInterPort(port);
			}

			XmlTok* ichubp;
			hubp->findChild("interconnect");
			while((ichubp = hubp->getNextChild())) {
				// we need both, so if one is missing, don't try
				// to connect
				if(!ichubp->findChild("host"))		continue;
				string host = ichubp->getNextChild()->getData();
				if(!ichubp->findChild("port"))		continue;
				int port = Util::toInt(ichubp->getNextChild()->getData());

				log(qstat, "\tConnecting to " + host + ':' + 
						Util::toString(port));
				hub->openInterConnection(host, port);
			}
		}
		XmlTok* passp;
		p->findChild("ihubpass");
		while((passp = p->getNextChild())) {
			InterHub::addPass(passp->getAttr("cid"),passp->getData());
		}
	}
	return 0;
}


static const char*const _help_str =
	"qhub is a distributed hub for the ADC protocol.\n"
	"Current command line parameters are limited because we are lazy.\n"
	"For hub settings, see Settings.xml, which should have been installed\n"
	"to $(prefix)/etc/qhub\n"
	"Options:\n"
	"  --help            print this help and exit\n"
	"  --version         print version information and exit\n"
	"  --errfile=FILE    errors will be appended to FILE (default stderr)\n"
	"  --statfile=FILE   status messages appeneded to FILE (default stdout)\n"
	"  -q                quiet mode; equivalent to --errfile=/dev/null --statfile=/dev/null\n";
static const char*const _version_str =
	PACKAGE "/" VERSION ", written by Walter Doekes (Sedulus),\n"
	"\tJohn Backstrand (sandos), and Matt Pearson (Pseudo)\n"
	"Homepage: http://ddc.berlios.de\n"
	"SVN:      svn://svn.berlios.de/ddc/qhub\n";

int Settings::parseArgs(int argc, char **argv) throw()
{
	StringList sl(argv+1, argv+argc);
	for(StringList::iterator i = sl.begin(); i < sl.end(); ++i) {
		if(*i == "--help") {
			//these two options should only appear as the sole argument
			assert(i == sl.begin() && sl.size() == 1);
			log(qstat, _help_str);
			exit(EXIT_SUCCESS);
		}
		if(*i == "--version") {
			assert(i == sl.begin() && sl.size() == 1);
			log(qstat, _version_str);
			exit(EXIT_SUCCESS);
		}
		if(!i->compare(0,11,"--statfile=")) {
			string tmp = i->substr(11);
			FILE* fp = fopen(tmp.c_str(), "at");
			if(!fp) {
				log(qerr, "could not open statfile \"" + tmp + '"');
				log(qerr, "\t" + Util::errnoToString(errno));
				continue;
			}
			qstat = fp;
		}
		if(!i->compare(0,10,"--errfile=")) {
			string tmp = i->substr(10);
			FILE* fp = fopen(tmp.c_str(), "at");
			if(!fp) {
				log(qerr, "could not open errfile \"" + tmp + '"');
				log(qerr, "\t" + Util::errnoToString(errno));
				continue;
			}
			qerr = fp;
		}
		if(*i == "-q") {
			qstat = fopen("/dev/null", "w");
			qerr = fopen("/dev/null", "w");
			assert(qstat && qerr);
		}
	}
	return 0;
}

