#include <Plugin.h>

#include "compat_hash_map.h"

using namespace qhub;

static hash_map<string, lt_dlhandle> modules;

void qhub::init ()
{
	lt_dlinit();
}

void qhub::deinit ()
{
	lt_dlexit();
}

void qhub::removeModule(const char* filename)
{
	if(modules.find(string(filename)) != modules.end()){
		modules.erase(string(filename));
	} else {
		fprintf(stderr, "Trying to unload non-existing module %s.\n", filename);
	}
}

void qhub::openModule(const char* filename)
{
	lt_dlhandle tmp = lt_dlopenext(filename);

	if(tmp != NULL){
		modules[string(filename)] = tmp; 
	} else {
		fprintf(stderr, "Dynamic linking failed for %s.\n", filename);
	}
}

static void qhub::loadFromModule(const lt_dlhandle)
{
	//load function pointers here.
}