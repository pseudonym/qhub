#include <dlfcn.h>

#include "PluginManager.h"
#include "Logs.h"

using namespace std;
using namespace qhub;

const string PluginManager::PLUGIN_EXTENSION = ".so";

bool PluginManager::open(string const& name, string const& insertBefore) throw()
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
				p->handle = h;
				Plugin::PluginStarted action;
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

bool PluginManager::remove(string const& name) throw()
{
	for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i){
		if((*i)->getId() == name) {
			// deinit self before others
			Plugin::PluginStopped action;
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

void PluginManager::removeAll() throw()
{
	while(!plugins.empty()) {
		// Call in reverse order (ugly hack to get iterator to work with erase)
		Plugins::iterator i = plugins.begin();
		for(unsigned j = 1; j < plugins.size(); ++j)
			++i;
		// Fire in reverse order
		Plugin::PluginStopped action;
		for(Plugins::reverse_iterator j = plugins.rbegin(); j != plugins.rend(); ++j)
			(*j)->on(action, *i);
		void* h = (*i)->handle;
		delete *i;
		dlclose(h); // close AFTER deleting, not while
		plugins.erase(i);
	}
}

bool PluginManager::has(string const& name) throw()
{
	for(iterator i = begin(); i != end(); ++i) {
		if((*i)->getId() == name)
			return true;
	}
	return false;
}
