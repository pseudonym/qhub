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
	bool success = false;
	lt_dlhandle h = lt_dlopenext(filename);

	if(h != NULL) {
		lt_ptr ptr;
		PLUGIN_START start = NULL;
		PLUGIN_STOP stop = NULL;
		if((ptr = lt_dlsym(h, "start")) != NULL)
			start = (PLUGIN_START)ptr;
		if(start && (ptr = lt_dlsym(h, "stop")) != NULL)
			stop = (PLUGIN_STOP)ptr;
		if(start && stop) {
			fprintf(stderr, "Loading plugin %s\n", filename);
			Plugin* p = (Plugin*)start();
			if(p) {
				fprintf(stderr, "Success!\n");
				p->start = start;
				p->stop = stop;
				p->name = filename;
				p->handle = h;
				p->on(STARTED, NULL);
				modules.push_back(p);
				success = true;
			}
		}
	}
				
	if(!success) {
		fprintf(stderr, "Dynamic linking failed for %s\n", filename);
	}
}

void qhub::Plugin::removeModule(const char* filename) throw()
{
	string tmp = filename;
	for(Plugins::iterator i = modules.begin(); i != modules.end(); ++i){
		if((*i)->name == tmp) {
			(*i)->on(STOPPED, NULL);
			(*i)->stop();
			modules.erase(i);
		}
	}
}

void Plugin::fire(Message m, ADC* client) throw()
{
	for(Plugins::iterator i = modules.begin(); i != modules.end(); ++i) {
		(*i)->on(m, client);
	}
}

/*
void qhub::Plugin::load(const char* name)
{
	lt_ptr p;
	if((p = lt_dlsym(handle, name)) != NULL) {
		fprintf(stderr, "Adding function '%s' to module '%s'.\n", name, this->name.c_str());
		functions[name] = p;
	} else {
		fprintf(stderr, "Failed loading function %s.\n", name);
	}
}

void qhub::Plugin::loadFromModule()
{
	//load function pointers here.
	load("foo");
	load("login");
	load("authenticated");
}
*/
