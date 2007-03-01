#ifndef QHUB_SETTINGS_H
#define QHUB_SETTINGS_H

#include <string>
#include <boost/program_options/variables_map.hpp>

#include "Singleton.h"
#include "Speaker.h"
#include "XmlTok.h"

namespace qhub {

class SettingsListener {
private:
	template <int i> struct X {};

public:
	typedef X<0> Load;

	virtual void on(Load, const boost::program_options::variables_map&) throw() = 0;

	// quiet, compiler
	virtual ~SettingsListener() {}
};

class Settings : public Singleton<Settings>, public Speaker<SettingsListener> {
public:
	XmlTok* getConfig(const std::string& name) throw();
	bool isValid() throw();
	void load() throw();
	void loadInteractive() throw();
	void save() throw();
	void parseArgs(int, char**);

private:
	friend class Singleton<Settings>;

	XmlTok* root;

	Settings() throw(io_error);
	~Settings() throw() { delete root; }
};

} // namespace qhub

#endif // QHUB_SETTINGS_H
