#include <Plugin.h>

using namespace qhub;

list<Plugin*> Plugin::modules;

void qhub::Plugin::init ()
{
	lt_dlinit();

	//add search-dir
	//lt_dladdsearchdir("./plugins/");
}

void qhub::Plugin::deinit()
{
	lt_dlexit();
}

void qhub::Plugin::removeModule(const char* filename)
{
	for(list<Plugin*>::iterator i=modules.begin(); i!=modules.end(); i++){
		if(strcmp((*i)->getName(), filename) == 0){
			modules.erase(i);
		}
	}
}

Plugin::Plugin(const char* n, const lt_dlhandle h) : name(string(n)), handle(h)
{
	fprintf(stderr, "Created module %s.\n", name.c_str());
}

Plugin::~Plugin()
{
	lt_dlclose(handle);
}

void qhub::Plugin::openModule(const char* filename)
{
	lt_dlhandle tmp = lt_dlopenext(filename);

	if(tmp != NULL){
		Plugin* t = new Plugin(filename, tmp);
		modules.push_back(t);
		t->loadFromModule();
	} else {
		fprintf(stderr, "Dynamic linking failed for %s.\n", filename);
	}
}

void qhub::Plugin::on(string const& what, ADC* client)
{
	for(list<Plugin*>::iterator i=modules.begin(); i!=modules.end(); i++){
		int (*f)(ADC*) = (int(*)(ADC*))(*i)->functions[what];
		f(client);
	}
}

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
