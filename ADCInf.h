// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADCINF_H_
#define _INCLUDED_ADCINF_H_

#include "Util.h"
#include "compat_hash_map.h"


namespace qhub {

class ADCClient;

class ADCInf {
public:
	ADCInf(ADCClient* p) throw() : parent(p) {};
	void setInf(StringList const& sl) throw();
	void setInf(string const& key, string const& val) throw();
	string const& getNewInf(string const& key) const throw();
	string const& getSetInf(string const& key) const throw();
	bool newInf(string const& key) const throw();
	string const& getFullInf() const throw() { return full; }
	string getChangedInf() throw();
private:
	void updateInf() throw();
	typedef hash_map<string, string> Inf;
	Inf current;
	Inf changes;
	string full;
	ADCClient* parent;
};

} //namespace qhub

#endif //_INCLUDED_ADCINF_H_
