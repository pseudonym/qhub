// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_USERINFO_H_
#define _INCLUDED_USERINFO_H_

#include <map>
#include <string>

#include "config.h"
#include "Util.h"
#include "Socket.h"
#include "ADC.h"
#include "Command.h"

namespace qhub {

#define UIID(a,b) ((((uint16_t)a & 255)<<8)|((uint16_t)b & 255))

class UserInfo {
public:
	typedef std::map<uint16_t, string> InfMap;
	typedef InfMap::const_iterator const_iterator;
	const_iterator begin() throw() { return infMap.begin(); };
	const_iterator end() throw() { return infMap.end(); };

	// Constructor
	UserInfo(const Command& c) throw() : cmd(c) {
		fromADC(cmd);
	}
	
	// ADC
	void fromADC(const Command& c) throw() {
		modified = true;
		for(Command::ConstParamIter i = c.begin(); i != c.end(); ++i) {
			if(i->length() >= 2) {
				set(*i);
			}
		}
	}

	Command const& toADC(sid_type sid) throw() {
		if(modified) {
			Command(get("HU").empty() ? 'B' : 'S', Command::INF, sid).swap(cmd);
			string temp(2, ' ');
			for(InfMap::const_iterator i = infMap.begin(); i != infMap.end(); ++i) {
				temp.clear();
				temp += (char)(i->first >> 8);
				temp += (char)(i->first & 0xFF);
				cmd << CmdParam(temp, i->second);
			}
			modified = false;
		}
		return cmd;
	}

	// Merge
	void update(UserInfo const& other) throw() {
		modified = true;
		// Insert all new keys
		for(InfMap::const_iterator i = other.infMap.begin(); i != other.infMap.end(); ++i) {
			infMap[i->first] = i->second;
		}
		// Remove all empty keys
		for(InfMap::iterator i = infMap.begin(); i != infMap.end(); ) {
			if(i->second.empty())
				infMap.erase(i++);
			else
				i++;
		}
	}

	// Setters
	void set(u_int16_t key, string const& val) throw() {
		modified = true;
		infMap[key] = val;
	}
	void set(char const* key, string const& val) throw() {
		assert(strlen(key) == 2);
		set(UIID(key[0], key[1]), val);
	}
	void set(string const& key, string const& val) throw() {
		assert(key.length() == 2);
		set(UIID(key[0], key[1]), val);
	}
	void set(string const& keyAndVal) throw() {
		assert(keyAndVal.length() >= 2);
		set(UIID(keyAndVal[0], keyAndVal[1]), keyAndVal.substr(2));
	}

	// Getters
	string const& get(u_int16_t key) const throw() {
		InfMap::const_iterator i = infMap.find(key);
		if(i != infMap.end())
			return i->second;
		return Util::emptyString;
	}
	string const& get(char const* key) const throw() {
		assert(strlen(key) == 2);
		return get(UIID(key[0], key[1]));
	}
	string const& get(string const& key) const throw() {
		assert(key.length() == 2);
		return get(UIID(key[0], key[1]));
	}

	// Delete
	bool del(u_int16_t key) throw() {
		InfMap::iterator i = infMap.find(key);
		if(i == infMap.end())
			return false;
		infMap.erase(i);
		return true;
	}
	bool del(char const* key) throw() {
		assert(strlen(key) == 2);
		return del(UIID(key[0], key[1]));
	}
	bool del(string const& key) throw() {
		assert(key.length() == 2);
		return del(UIID(key[0], key[1]));
	}

	// Check
	bool has(u_int16_t key) const throw() {
		InfMap::const_iterator i = infMap.find(key);
		if(i == infMap.end() || i->second.empty())
			return false;
		return true;
	}
	bool has(char const* key) const throw() {
		assert(strlen(key) == 2);
		return has(UIID(key[0], key[1]));
	}
	bool has(string const& key) const throw() {
		assert(key.length() == 2);
		return has(UIID(key[0], key[1]));
	}

	// Quick access
	string const& getNick() const throw() {
		return get(UIID('N','I'));
	}
	string const& getCID() const throw() {
		return get(UIID('I','D'));
	}
	int const getOp() const throw() {
		return get("OP").empty() ? 0 : 1;
	}
	bool const hasSupport(const string& feat) const throw()
	{
		const StringList& sl = Util::stringTokenize(get(UIID('S','U')), ',');
		return find(sl.begin(), sl.end(), feat) != sl.end();
	}

private:
	InfMap infMap;
	bool modified;
	Command cmd;

	UserInfo() throw();
};

} //namespace qhub

#endif //_INCLUDED_USERINFO_H_
