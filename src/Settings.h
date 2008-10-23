#ifndef QHUB_SETTINGS_H
#define QHUB_SETTINGS_H

#include "qhub.h"
#include "Singleton.h"
#include "XmlTok.h"

#include <string>

namespace qhub {

class Settings : public Singleton<Settings> {
public:
	XmlTok* getConfig(const std::string& name) throw();
	std::string getConfigDir() throw() { return "."; }
	bool isValid() throw();
	void load() throw();
	void loadInteractive() throw();
	void save() throw();
	void parseArgs(int, char**);

private:
	friend class Singleton<Settings>;

	XmlTok* root;
	std::string configDir;

	Settings() throw() : root(NULL) {}
	~Settings() throw() { delete root; }
};

} // namespace qhub

#endif // QHUB_SETTINGS_H
