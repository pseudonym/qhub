// vim:ts=4:sw=4:noet
#include <cassert>
#include <iostream>

#include "error.h"

#include "Settings.h"
#include "Hub.h"
#include "XmlTok.h"
#include "Util.h"
#include "Logs.h"
#include "TigerHash.h"
#include "Encoder.h"

using namespace qhub;
using namespace std;

XmlTok Settings::root;

XmlTok* Settings::getConfig(const string& name) throw()
{
	XmlTok* p = (root.findChild("qhub"), root.getNextChild());
	assert(p);
	if(p->findChild(name))
		return p->getNextChild();
	else 
		return p->addChild(name);
}

bool Settings::isValid() throw()
{
	XmlTok* p;
	return root.findChild("qhub")
	    && (p = root.getNextChild())
	    && p->findChild("__core")
	    && (p = p->getNextChild())
	    && ADC::checkCID(p->getAttr("cid")); // enough checks for now :)
}

void Settings::load() throw()
{
	if(!root.load(CONFIGDIR "/qhub.xml") || !isValid()) {
		Logs::err << "Config file missing or corrupt, entering interactive setup\n";
		loadInteractive();
	}

	XmlTok* p = getConfig("__core");

	string name, cid;
	name = p->getAttr("name");
	Logs::stat << "Hub Name: " << name << endl;
	cid = p->getAttr("cid");

	Hub* hub = new Hub(cid, name);
	hub->setInterPass(p->getAttr("interpass"));

	XmlTok* pp;

	p->findChild("clientport");
	while((pp = p->getNextChild())) {
		int port = Util::toInt(pp->getData());
		Logs::stat << "Client Port: " << port << '\n';
		if(port > 0 && port <= 65535)
			hub->openClientPort(port);
	}

	p->findChild("interport");
	while((pp = p->getNextChild())) {
		int port = Util::toInt(pp->getData());
		Logs::stat << "Inter-hub Port: " << port << '\n';
		if(port > 0 && port <= 65535)
			hub->openInterPort(port);
	}

	p->findChild("interconnect");
	while((pp = p->getNextChild())) {
		string host = pp->getAttr("host");
		int port = Util::toInt(pp->getAttr("port"));
		if(host.empty() || port <= 0 || port > 65535)
			continue;
		Logs::stat << "Connecting to " << host << ':' << port << endl; 
		hub->openInterConnection(host, port);
	}
}

void Settings::loadInteractive() throw()
{
	string name, interpass, cid32;
	vector<u_int16_t> cports, iports;
	cout << "Hub name: ";
	getline(cin, name);
	cout << "Client ports (0 when done): ";
	while(cin) {
		int port;
		cin >> port;
		if(!port)
			break;
		cports.push_back(port);
	}
	cout << "Interconnect ports (0 when done): ";
	while(cin) {
		int port;
		cin >> port;
		if(!port)
			break;
		iports.push_back(port);
	}
	if(!iports.empty()) {
		cout << "Interconnect password: ";
		cin >> interpass;
	}
	cout << "Generating CID..." << endl;
	// throw a bunch of data at Tiger, then take first 64 bits
	// not to spec, but should be effective
	TigerHash th;
	th.update(name.data(), name.size());
	time_t t = time(NULL);
	th.update(&t, sizeof(time_t));
	if(!cports.empty())
		th.update(&cports.front(), cports.size()*sizeof(u_int16_t));
	if(!iports.empty())
		th.update(&iports.front(), iports.size()*sizeof(u_int16_t));
	th.update(interpass.data(), interpass.size());
	th.update(&Util::genRand(24).front(), 24);
	th.finalize();
	Encoder::toBase32(th.getResult(), 64/8, cid32);

	root.clear();
	XmlTok* p = root.addChild("qhub");
	p = p->addChild("__core");
	p->setAttr("name", name);
	p->setAttr("cid", cid32);
	if(!interpass.empty())
		p->setAttr("interpass", interpass);
	for(vector<u_int16_t>::iterator i = cports.begin(); i != cports.end(); ++i)
		p->addChild("clientport")->setData(Util::toString(*i));
	for(vector<u_int16_t>::iterator i = iports.begin(); i != iports.end(); ++i)
		p->addChild("interport")->setData(Util::toString(*i));
}

void Settings::save() throw()
{
	if(!root.save(CONFIGDIR "/qhub.xml"))
		Logs::err << "Unable to save config file, check write permissions" << endl;
}

/*
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
		if(hubp->findChild("maxpacketsize")) {
			int size = Util::toInt(hubp->getNextChild()->getData());
			Logs::stat << "Max Packet Size: " << size << '\n';
			if(size > 0)
				hub->setMaxPacketSize(size);
		}
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
}*/


static const char*const _help_str =
PACKAGE_NAME " is a distributed hub for the ADC protocol.\n"
"Current command line parameters are limited because we are lazy.\n"
"For hub settings, see Settings.xml, which should have been installed\n"
"to " CONFIGDIR "\n"
"Options:\n"
"  --help            print this help and exit\n"
"  --version, -V     print version information and exit\n"
"  --errfile=FILE    errors will be appended to FILE (default stderr)\n"
"  --statfile=FILE   status messages appended to FILE (default stdout)\n"
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

