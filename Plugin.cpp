// vim:ts=4:sw=4:noet
#include <Plugin.h>

using namespace qhub;

UserData Plugin::data;
Plugin::Plugins Plugin::plugins;

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

bool Plugin::openModule(string const& filename, string const& insertBefore) throw()
{
	lt_dlhandle h = lt_dlopenext(filename.c_str());

	if(h != NULL) {
		lt_ptr ptr;
		if((ptr = lt_dlsym(h, "getPlugin")) != NULL) {
			get_plugin_t getPlugin = (get_plugin_t)ptr;
			Plugin* p = (Plugin*)getPlugin();
			if(p) {
				p->name = filename;
				p->handle = h;
//				p->on(PluginStarted(), p); // init self before others
//				fire(PluginStarted(), p);
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
				fprintf(stderr, "Loading plugin \"%s\" SUCCESS!\n", filename.c_str());
				return true;
			}	
		}
	}
	fprintf(stderr, "Loading plugin \"%s\" FAILED!\n", filename.c_str());
	return false;
}

bool Plugin::removeModule(string const& filename) throw()
{
	for(Plugins::iterator i = plugins.begin(); i != plugins.end(); ++i){
		if((*i)->name == filename) {
			// deinit self before others
//			(*i)->on(PluginStopped(), *i);
			lt_dlhandle h = (*i)->handle;
			delete *i;
			lt_dlclose(h); // close AFTER deleting, not while
			plugins.erase(i);
			// fire in reverse order
//			for(Plugins::reverse_iterator j = plugins.rbegin(); j != plugins.rend(); ++j)
//				(*j)->on(PluginStopped(), *i);
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
//		for(Plugins::reverse_iterator j = plugins.rbegin(); j != plugins.rend(); ++j)
//			(*j)->on(PluginStopped(), *i);
		lt_dlhandle h = (*i)->handle;
		delete *i;
		lt_dlclose(h); // close AFTER deleting, not while
		plugins.erase(i);
	}
}

bool Plugin::hasModule(string const& filename) throw()
{
	for(iterator i = begin(); i != end(); ++i) {
		if((*i)->getId() == filename)
			return true;
	}
	return false;
}
