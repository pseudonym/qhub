// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_USERINFO_H_
#define _INCLUDED_USERINFO_H_

#include "Util.h"
#include "Socket.h"

namespace qhub {

#define UIID(a,b) ((u_int16_t)(((char)a<<8)|(char)b))

class UserInfo {
public:
	typedef map<u_int16_t, string> InfMap;
	typedef map<u_int16_t, string>::const_iterator const_iterator;
	const_iterator begin() throw() { return infMap.begin(); };
	const_iterator end() throw() { return infMap.end(); };

	// Constructor
	UserInfo(Socket* p) : sock(p) {};
	
	// ADC
	void fromADC(StringList& sl) throw() {
		for(StringList::const_iterator sli = sl.begin() + 2; sli != sl.end(); ++sli) {
			if(sli->length() >= 2) {
				set(*sli);
			}
		}
	}
	string const& toADC(string const& cid32) throw() {
		if(modified) {
			adcString = "BINF ";
			adcString += cid32;
			string temp;
			for(InfMap::const_iterator i = infMap.begin(); i != infMap.end(); ++i) {
				temp.clear();
				temp += (char)(i->first >> 8);
				temp += (char)(i->first & 0xFF);
				temp += i->second;
				adcString += ' ' + /*ADCClient::esc(*/temp;//);
			}
			adcString += '\n';
			modified = false;
		}
		return adcString;
	}

	// Merge
	void update(UserInfo const& other) throw() {
		modified = true;
		// Insert all new keys
		for(InfMap::const_iterator i = other.infMap.begin(); i != other.infMap.end(); ++i) {
			infMap[i->first] = i->second;
		}
		// Remove all empty keys
		for(bool done = false; !done && (done = true);) {
			for(InfMap::iterator i = infMap.begin(); i != infMap.end(); ++i) {
				if(i->second.empty()) {
					infMap.erase(i);
					done = false;
					break;
				}
			}
		}
	}

	// Setters
	void set(u_int16_t key, string const& val) throw() {
		modified = true;
		// Fix I4 and I6
		if(key == UIID('I','4') && val == "0.0.0.0") {
			if(sock->getDomain() == PF_INET)
				infMap[key] = sock->getPeerName();
		} else if(key == UIID('I','6') && val == "[0:0:0:0:0:0:0:0]") {
			if(sock->getDomain() == PF_INET6)
				infMap[key] = sock->getPeerName();
		} else {
			infMap[key] = val;
		}
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
	int const getOp() const throw() {
		return Util::toInt(get(UIID('O','P')));
	}
	bool const isUdpActive() const throw() {
		return has(UIID('U','4')) || has(UIID('U','6'));
	}
		
private:
	Socket* sock;
	InfMap infMap;
	bool modified;
	string adcString;
	
};

} //namespace qhub

#endif //_INCLUDED_USERINFO_H_
