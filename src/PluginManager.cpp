// vim:ts=4:sw=4:noet
#include "PluginManager.h"

#include "Logs.h"

#include <stdexcept>

#include <dlfcn.h>

using namespace std;
using namespace qhub;

namespace {

const string PLUGIN_EXTENSION = ".so";

typedef Plugin* (*GetPlugin)();
typedef void (*PutPlugin)(Plugin*);

} // anon namespace

bool PluginManager::open(string const& name, string const& insertBefore) throw()
{
#define CHECKERR() if(const char* err = dlerror()) throw runtime_error(err)
	try {
		const string filename = "qhub-" + name + PLUGIN_EXTENSION;
		void* h = dlopen(filename.c_str(), RTLD_GLOBAL | RTLD_LAZY);
		CHECKERR();

		GetPlugin getPlugin = (GetPlugin) dlsym(h, "getPlugin");
		CHECKERR();

		PutPlugin putPlugin = (PutPlugin) dlsym(h, "putPlugin");
		CHECKERR();

		if(!getPlugin || !putPlugin)
			throw runtime_error("get and/or put functions are NULL");

		Plugin* p = getPlugin();
		p->handle = h;

		Plugin::PluginStarted action;
		p->on(action, p); // init self before others
		fire(action, p);

		if(insertBefore.empty()) {
			plugins.push_back(p);
		} else {
			iterator i;
			for(i = begin(); i != end(); ++i) {
				if((*i)->getId() == insertBefore)
					break;
			}
			plugins.insert(i, p);
		}
		Logs::stat << "Loading plugin \"" << name << "\" SUCCESS!\n";
		return true;
#undef CHECKERR
	} catch(const runtime_error& e) {
		Logs::err << "Loading plugin \"" << name << "\" FAILED! " << e.what() << endl;
		return false;
	}
}

bool PluginManager::remove(string const& name) throw()
{
	for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i){
		if((*i)->getId() == name) {
			// fire in reverse order
			Plugin::PluginStopped action;
			for(Plugins::reverse_iterator j = plugins.rbegin(); j != plugins.rend(); ++j)
				(*j)->on(action, *i);

			Plugin* p = *i;
			plugins.erase(i);
			void* h = p->handle;

			// clear errors just in case
			dlerror();

			PutPlugin putPlugin = (PutPlugin) dlsym(h, "putPlugin");
			assert(dlerror() == NULL); // better not happen... it worked at open
			putPlugin(p);

			dlclose(h); // close AFTER deleting, not while
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
