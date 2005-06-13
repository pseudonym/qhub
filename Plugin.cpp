// vim:ts=4:sw=4:noet
#include "Plugin.h"
#include "Util.h"
#include <dlfcn.h>
#include "error.h"

using namespace qhub;

UserData Plugin::data;
Plugin::Plugins Plugin::plugins;
const string Plugin::PLUGIN_EXTENSION = ".so";

void Plugin::init() throw()
{
}

void Plugin::deinit() throw()
{
	removeAllModules();
}

bool Plugin::openModule(string const& name, string const& insertBefore) throw()
{
	string filename = "qhub-" + name + PLUGIN_EXTENSION;
	void* h = dlopen(filename.c_str(), RTLD_GLOBAL | RTLD_LAZY);
	char const* error;

	if(h != NULL) {
		void* ptr = dlsym(h, "getPlugin");
		if((error = dlerror()) == NULL) {
			get_plugin_t getPlugin = (get_plugin_t)ptr;
			Plugin* p = (Plugin*)getPlugin();
			if(p) {
				p->name = name;
				p->handle = h;
				PluginStarted action;
				p->on(action, p); // init self before others
				fire(action, p);
				if(insertBefore.empty()) {
					plugins.push_back(p);
				} else {
					iterator j = begin();
					for(iterator i = begin(); i != end(); j = ++i) {
						if((*i)->getId() == insertBefore)
							break;
					}
					plugins.insert(j, 1, p);
				}
				Logs::stat << "Loading plugin \"" << name << "\" SUCCESS!\n";
				return true;
			}
		}
	} else {
		error = dlerror();
	}
	Logs::err << "Loading plugin \"" << name << "\" FAILED! " <<
			(error != NULL ? error : "") << endl;
	return false;
}

bool Plugin::removeModule(string const& name) throw()
{
	for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i){
		if((*i)->name == name) {
			// deinit self before others
			PluginStopped action;
			(*i)->on(action, *i);
			void* h = (*i)->handle;
			delete *i;
			dlclose(h); // close AFTER deleting, not while
			plugins.erase(i);
			// fire in reverse order
			for(Plugins::reverse_iterator j = plugins.rbegin(); j != plugins.rend(); ++j)
				(*j)->on(action, *i);
			return true;
		}
	}
	return false;
}

void Plugin::removeAllModules() throw()
{
	while(!plugins.empty()) {
		// Call in reverse order (ugly hack to get iterator to work with erase)
		Plugins::iterator i = plugins.begin();
		for(unsigned j = 1; j < plugins.size(); ++j)
			++i;
		// Fire in reverse order
		PluginStopped action;
		for(Plugins::reverse_iterator j = plugins.rbegin(); j != plugins.rend(); ++j)
			(*j)->on(action, *i);
		void* h = (*i)->handle;
		delete *i;
		dlclose(h); // close AFTER deleting, not while
		plugins.erase(i);
	}
}

bool Plugin::hasModule(string const& name) throw()
{
	for(iterator i = begin(); i != end(); ++i) {
		if((*i)->getId() == name)
			return true;
	}
	return false;
}
