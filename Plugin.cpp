#include <Plugin.h>

using namespace qhub;

Plugin::Plugins Plugin::modules;

void qhub::Plugin::init() throw()
{
	lt_dlinit();
	//add search-dir
	//lt_dladdsearchdir("./plugins/");
}

void qhub::Plugin::deinit() throw()
{
	lt_dlexit();
}

Plugin::~Plugin() throw()
{
	lt_dlclose(handle);
}

void qhub::Plugin::openModule(const char* filename) throw()
{
	fprintf(stderr, "Loading plugin %s ... ", filename);
	lt_dlhandle h = lt_dlopenext(filename);

	if(h != NULL) {
		lt_ptr ptr;
		if((ptr = lt_dlsym(h, "getPlugin")) != NULL) {
			get_plugin_t getPlugin = (get_plugin_t)ptr;
			Plugin* p = (Plugin*)getPlugin();
			if(p) {
				fprintf(stderr, "success\n");
				p->name = filename;
				p->handle = h;
				p->on(STARTED, NULL, Util::emptyString);
				modules.push_back(p);
				return;
			}	
		}
	}
	fprintf(stderr, "failed\n");
}

void qhub::Plugin::removeModule(const char* filename) throw()
{
	string tmp = filename;
	for(Plugins::iterator i = modules.begin(); i != modules.end(); ++i){
		if((*i)->name == tmp) {
			(*i)->on(STOPPED, NULL, Util::emptyString);
			delete *i;
			modules.erase(i);
		}
	}
}

void Plugin::fire(Message m, ADC* client, string const& msg) throw()
{
	for(Plugins::iterator i = modules.begin(); i != modules.end(); ++i) {
		(*i)->on(m, client, msg);
	}
}
