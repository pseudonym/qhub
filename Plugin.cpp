#include <Plugin.h>

using namespace qhub;

Plugin::Plugins Plugin::modules;

void Plugin::init() throw()
{
	lt_dlinit();
	//add search-dir
	//lt_dladdsearchdir("./plugins/");
}

void Plugin::deinit() throw()
{
	removeAllModules();
	lt_dlexit();
}

void Plugin::openModule(const char* filename) throw()
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

void Plugin::removeModule(const char* filename) throw()
{
	string tmp = filename;
	for(Plugins::iterator i = modules.begin(); i != modules.end(); ++i){
		if((*i)->name == tmp) {
			(*i)->on(STOPPED, NULL, Util::emptyString);
			lt_dlhandle h = (*i)->handle;
			delete *i;
			lt_dlclose(h); // close AFTER deleting, not while
			modules.erase(i);
		}
	}
}

void Plugin::removeAllModules() throw()
{
	while(!modules.empty()) {
		Plugins::iterator i = modules.begin();
		(*i)->on(STOPPED, NULL, Util::emptyString);
		lt_dlhandle h = (*i)->handle;
		delete *i;
		lt_dlclose(h); // close AFTER deleting, not while
		modules.erase(i);
	}
}	

void Plugin::fire(Message m, ADCClient* client, string const& msg) throw()
{
	for(Plugins::iterator i = modules.begin(); i != modules.end(); ++i) {
		(*i)->on(m, client, msg);
	}
}
