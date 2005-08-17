#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#include <string>

namespace qhub {

class Settings {
public:
	static std::string getFilename(const std::string& name) throw()
	{
		return CONFIGDIR "/" + name + ".xml";
	}
	static void readFromXML();
	static void parseArgs(int, char**);
};

}

#endif
