// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_ADCINF_H_
#define _INCLUDED_ADCINF_H_

#include "Util.h"
#include "compat_hash_map.h"


namespace qhub {

class ADC;

class ADCInf {
public:
	ADCInf(ADC* p) throw() : parent(p) {};
	bool setInf(StringList const& sl) throw();
	bool setInf(string const& key, string const& val) throw();
	string const& getInf(string const& key) const throw();
	string const& getFullInf() const throw() { return full; }
	string getChangedInf() throw();
private:
	void updateInf() throw();
	typedef hash_map<string, string> Inf;
	Inf current;
	Inf changes;
	string full;
	ADC* parent;
};

} //namespace qhub

#endif //_INCLUDED_ADCINF_H_
