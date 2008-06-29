// vim:ts=4:sw=4:noet
#include "Settings.h"

#include "Logs.h"
#include "PluginManager.h"
#include "Util.h"
#include "XmlTok.h"

#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

using namespace qhub;
using namespace std;

Settings::Settings() throw(io_error) : root(NULL)
{
	try {
		ifstream f(CONFIGDIR "/qhub.xml");
		if(!f.good())
			throw io_error("could not load config file");
		root = new XmlTok(f);
	} catch(const io_error& e) {
		// nothing to do; won't be valid so it will enter
		// interactive load
	}
	load();
}

XmlTok* Settings::getConfig(const string& name) throw()
{
	if(root->findChild(name))
		return root->getNextChild();
	else 
		return root->addChild(name);
}

bool Settings::isValid() throw()
{
	return true
		&& root
		&& root->getName() == "qhub"
	    && root->findChild("__connections")
	    && root->findChild("__hub")
	    ;
}

void Settings::load() throw()
{
	if(!isValid()) {
		Logs::err << "Config file missing or corrupt, entering interactive setup\n";
		loadInteractive();
	}
}

void Settings::loadInteractive() throw()
{
	string name, interpass, prefix;
	vector<uint16_t> cports, iports;
	cout << "Hub name: ";
	getline(cin, name);
	cout << "Client ports (0 when done): ";
	while(cin) {
		int port = 0;
		cin >> port;
		if(!port)
			break;
		cports.push_back(port);
	}
	cout << "Interconnect ports (0 when done): ";
	while(cin) {
		int port = 0;
		cin >> port;
		if(!port)
			break;
		iports.push_back(port);
	}
	if(!iports.empty()) {
		cout << "Interconnect password: ";
		cin >> interpass;
	}
	cout << "number of bits to be used for hub identification on this network "
			<< "(0 if this hub will not be part of a network): ";
	int bits;
	cin >> bits;
	int id = 0;
	while(bits != 0) {
		cout << "hub id number for the network (0 for no network): ";
		cin >> id;
		if(id < (1 << bits))
			break;
		cerr << "hub id is too large for number of bits!" << endl;
	}

	delete root;
	root = new XmlTok("qhub");
	XmlTok* p = root->addChild("__hub");
	p->setAttr("name", name);
	p->setAttr("hubsidbits", Util::toString(bits));
	p->setAttr("sid", Util::toString(id << (20 - bits)));
	if(!interpass.empty())
		p->setAttr("interpass", interpass);

	p = root->addChild("__connections");
	for(vector<uint16_t>::iterator i = cports.begin(); i != cports.end(); ++i)
		p->addChild("clientport")->setData(Util::toString(*i));
	for(vector<uint16_t>::iterator i = iports.begin(); i != iports.end(); ++i)
		p->addChild("interport")->setData(Util::toString(*i));
}

void Settings::save() throw()
{
	try {
		ofstream f(CONFIGDIR "/qhub.xml");
		root->save(f);
	} catch(const io_error& e) {
		Logs::err << "Unable to save qhub.xml: " << e.what() << endl;
	}
}

static const char*const _version_str =
PACKAGE_NAME "/" PACKAGE_VERSION " built " __DATE__ "\n"
"Written by Walter Doekes (Sedulus),\n"
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
		("plugin,p", value<StringList>(), "load plugin 'arg'")
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
#ifdef DEBUG
	if(vm.count("verbose"))
		Logs::copy(Logs::stat, Logs::line);
#endif
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
	if(vm.count("plugin")) {
		const StringList& p = vm["plugin"].as<StringList>();
		for(StringList::const_iterator i = p.begin(); i != p.end(); ++i)
			PluginManager::instance()->open(*i);
	}
	if(vm.count("daemonize"))
		// TODO check to make sure config is done and valid
		Util::daemonize();
}
