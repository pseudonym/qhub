// vim:ts=4:sw=4:noet
#include <cassert>

#include "error.h"

#include "Settings.h"
#include "Hub.h"
#include "XmlTok.h"
#include "Util.h"
#include "Logs.h"

using namespace qhub;
using namespace std;

void Settings::readFromXML()
{
	XmlTok root;
	if(!root.load(getFilename("Settings")))
		throw Exception("cannot open config file");

	XmlTok* p = &root;
	if(!p->findChild("config")) {
		Logs::err << "configuration file corrupt: no <config> tag" << endl;
		return;
	}
	p = p->getNextChild();

	// list all hubs
	XmlTok* hubp;
	p->findChild("hub");
	while((hubp = p->getNextChild())) {
		string name, cid;
		if(hubp->findChild("name")) {
			name = hubp->getNextChild()->getData();
			Logs::stat << "\nHubname: " << name << endl;
		} else {
			Logs::err << "All hubs must have names: skipping one\n";
			continue;
		}
		if(hubp->findChild("cid")) {
			cid = hubp->getNextChild()->getData();
			Logs::stat << "Hub CID: " << cid << '\n';
		} else {
			Logs::err << "All hubs must have CIDs: skipping one\n";
			continue;
		}

		Hub* hub = new Hub(cid, name);

		if(hubp->findChild("description")) {
			string desc = hubp->getNextChild()->getData();
			Logs::stat << "Hub description: " << desc << '\n';
			hub->setDescription(desc);
		}
		if(hubp->findChild("port")) {
			int port = Util::toInt(hubp->getNextChild()->getData());
			Logs::stat << "Client Port: " << port << '\n';
			if(port > 0 && port <= 65535)
				hub->openClientPort(port);
		}
		/*
		if(hubp->findChild("maxpacketsize")) {
			int size = Util::toInt(hubp->getNextChild()->getData());
			Logs::stat << "Max Packet Size: " << size << '\n';
			if(size > 0)
				hub->setMaxPacketSize(size);
		}*/
		if(hubp->findChild("interport")) {
			int port = Util::toInt(hubp->getNextChild()->getData());
			Logs::stat << "Inter-hub Port: " << port << '\n';
			if(port > 0 && port <= 65535)
				hub->openInterPort(port);
		}
		if(hubp->findChild("interpass")) {
			string pass = hubp->getNextChild()->getData();
			Logs::stat << "Inter-hub Pass: " << pass << '\n';
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
			if(!ichubp->findChild("port")) {
				Logs::err << "Interconnect port not specified.. skipping\n";
				continue;
			}
			int port = Util::toInt(ichubp->getNextChild()->getData());
			Logs::stat << "Connecting to " << host << ':' << port << endl; 
			hub->openInterConnection(host, port);
		}
	}
}


static const char*const _help_str =
PACKAGE_NAME " is a distributed hub for the ADC protocol.\n"
"Current command line parameters are limited because we are lazy.\n"
"For hub settings, see Settings.xml, which should have been installed\n"
"to " CONFIGDIR "\n"
"Options:\n"
"  --help            print this help and exit\n"
"  --version, -V     print version information and exit\n"
"  --errfile=FILE    errors will be appended to FILE (default stderr)\n"
"  --statfile=FILE   status messages appeneded to FILE (default stdout)\n"
"  --linefile=FILE   protocol lines appended to FILE (default /dev/null, '-'=same as statfile)\n"
"  -q                quiet mode; equivalent to --errfile=/dev/null --statfile=/dev/null\n"
"  -v                verbose mode; equivalent to --linefile=-\n";

static const char*const _version_str =
PACKAGE_NAME "/" PACKAGE_VERSION ", written by Walter Doekes (Sedulus),\n"
"\tJohn Backstrand (sandos), and Matt Pearson (Pseudo)\n"
"Homepage: http://ddc.berlios.de\n"
"SVN:      svn://svn.berlios.de/ddc/qhub\n";

void Settings::parseArgs(int argc, char **argv)
{
	StringList sl(argv+1, argv+argc);
	for(StringList::iterator i = sl.begin(); i < sl.end(); ++i) {
		if(*i == "--help") {
			//these two options should only appear as the sole argument
			assert(i == sl.begin() && sl.size() == 1);
			Logs::stat << _help_str;
			exit(EXIT_SUCCESS);
		}
		else if(*i == "--version" || *i == "-V") {
			assert(i == sl.begin() && sl.size() == 1);
			Logs::stat << _version_str;
			exit(EXIT_SUCCESS);
		}
		else if(!i->compare(0,11,"--statfile=")) {
			Logs::set(Logs::stat, i->substr(11));
		}
		else if(!i->compare(0,11,"--linefile=")) {
			string tmp(*i, 11);
			if(tmp == "-") {
				Logs::copy(Logs::stat, Logs::line);
			} else {
				Logs::set(Logs::line, tmp);
			}
		}
		else if(!i->compare(0,10,"--errfile=")) {
			Logs::set(Logs::err, i->substr(10));
		}
		else if(*i == "-q") {
			Logs::copy(Logs::line, Logs::stat);
			Logs::copy(Logs::line, Logs::err);
		}
		else if(*i == "-v") {
			Logs::copy(Logs::stat, Logs::line);
		}
	}
}

