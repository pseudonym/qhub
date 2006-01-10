#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#include <string>
#include "XmlTok.h"

namespace qhub {

class Settings {
	static XmlTok root;
public:
	static XmlTok* getConfig(const std::string& name) throw();
	static bool isValid() throw();
	static void load() throw();
	static void loadInteractive() throw();
	static void save() throw();
	//static void readFromXML();
	static void parseArgs(int, char**);
};

}

#endif
