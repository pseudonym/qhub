// vim:ts=4:sw=4:noet
#include <cassert>
#include <iostream>
#include <boost/program_options.hpp>

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

static const char*const _version_str =
PACKAGE_NAME "/" PACKAGE_VERSION ", written by Walter Doekes (Sedulus),\n"
"\tJohn Backstrand (sandos), and Matt Pearson (Pseudo)\n"
"Homepage: http://ddc.berlios.de\n"
"SVN:      svn://svn.berlios.de/ddc/qhub\n";

void Settings::parseArgs(int argc, char** argv)
{
	using namespace boost::program_options;

	options_description desc("Options");
	desc.add_options()
		("help", "display this help and exit")
		("version", "display version information and exit")
		("statfile", value<string>(), "status messages logged to 'arg' (default stdout)")
		("errfile", value<string>(), "error messages logged to 'arg' (default stderr)")
#ifdef DEBUG
		("linefile", value<string>(), "protocol lines logged to 'arg'")
		("verbose,v", "protocol lines logged to stdout (only for debugging)")
#endif
		("daemonize,d", "run as daemon")
		("quiet,q", "no output");

	variables_map vm;
	store(parse_command_line(argc, argv, desc), vm);
	notify(vm);

	if(vm.count("help")) {
		cout << desc;
		exit(EXIT_SUCCESS);
	}
	if(vm.count("version")) {
		cout << _version_str;
		exit(EXIT_SUCCESS);
	}
	if(vm.count("verbose"))
		Logs::copy(Logs::stat, Logs::line);
	if(vm.count("statfile"))
		Logs::setStat(vm["statfile"].as<string>());
#ifdef DEBUG
	if(vm.count("linefile"))
		Logs::setLine(vm["statfile"].as<string>());
#endif
	if(vm.count("errfile"))
		Logs::setErr(vm["errfile"].as<string>());
	if(vm.count("quiet")) {
#ifdef DEBUG
		Logs::line.rdbuf(0);
#endif
		Logs::stat.rdbuf(0);
		Logs::err.rdbuf(0);
	}
	if(vm.count("daemonize"))
		// TODO check to make sure config is done and valid
		Util::daemonize();
}
