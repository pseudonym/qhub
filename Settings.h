#ifndef __SETTINGS_H_
#define __SETTINGS_H_

namespace qhub {

class Settings {
public:
	static int readFromXML() throw();
	static int parseArgs(int, char**) throw();
};

}

#endif
