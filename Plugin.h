#ifndef __PLUGIN_H_
#define __PLUGIN_H_

#include <string>
#include <ltdl.h>

#include <compat_hash_map.h>


using namespace std;

namespace qhub {


void init();
void deinit();

void openModule(const char* filename);
void removeModule(const char* filename);





static void loadFromModule(const lt_dlhandle);

}


#endif
