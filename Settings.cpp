// vim:ts=4:sw=4:noet
#include <cstdio>
#include <cassert>

#include "error.h"

#include "Settings.h"
#include "Hub.h"
#include "XmlTok.h"
#include "Util.h"
#include "Logs.h"

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
				Logs::stat << "Hubname: " << name << endl;
				hub->setHubName(name);
			} else {
				Logs::err << "All hubs must have names: skipping one\n";
				delete hub;
				continue;
			}
			if(hubp->findChild("cid")) {
				string cid = hubp->getNextChild()->getData();
				log(qstat, "\tHub CID: " + cid);
				hub->setCID32(cid);
			} else {
				Logs::err << "All hubs must have CIDs: skipping one\n";
				delete hub;
				continue;
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
			if(hubp->findChild("interpass")) {
				string pass = hubp->getNextChild()->getData();
				log(qstat, "\tInter-hub Pass: " + pass);
				hub->setInterPass(pass);
			}

			XmlTok* ichubp;
			hubp->findChild("interconnect");
			while((ichubp = hubp->getNextChild())) {
				// we need both, so if one is missing, don't try
				// to connect
				if(!ichubp->findChild("host")) {
					Logs::err << "Interconnect host not specified.. skipping\n";
					continue;
				}
				string host = ichubp->getNextChild()->getData();
				if(!ichubp->findChild("port"))		continue;
				int port = Util::toInt(ichubp->getNextChild()->getData());

				Logs::stat << "\tConnecting to " << host << ':' << port << endl; 
				hub->openInterConnection(host, port);
			}
		}
	}
	return 0;
}


static const char*const _help_str =
PACKAGE " is a distributed hub for the ADC protocol.\n"
"Current command line parameters are limited because we are lazy.\n"
"For hub settings, see Settings.xml, which should have been installed\n"
"to " CONFIGDIR "\n"
"Options:\n"
"  --help            print this help and exit\n"
"  --version         print version information and exit\n"
"  --errfile=FILE    errors will be appended to FILE (default stderr)\n"
"  --statfile=FILE   status messages appeneded to FILE (default stdout)\n"
"  --linefile=FILE   protocol lines appended to FILE (default /dev/null, '-'=same as statfile)\n"
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
			Logs::stat << _help_str;
			exit(EXIT_SUCCESS);
		}
		if(*i == "--version") {
			assert(i == sl.begin() && sl.size() == 1);
			Logs::stat << _version_str;
			exit(EXIT_SUCCESS);
		}
		if(!i->compare(0,11,"--statfile=")) {
			Logs::setStat(i->substr(11));
		}
		if(!i->compare(0,11,"--linefile=")) {
			string tmp(*i, 11);
			if(tmp == "-") {
				Logs::copy(Logs::stat, Logs::line);
			} else {
				Logs::setLine(tmp);
			}
		}
		if(!i->compare(0,10,"--errfile=")) {
			Logs::setErr(i->substr(10));
		}
		if(*i == "-q") {
			Logs::copy(Logs::line, Logs::stat);
			Logs::copy(Logs::line, Logs::err);
		}
	}
	return 0;
}

