// vim:ts=4:sw=4:noet
#ifndef _INCLUDED_XMLTOK_H_
#define _INCLUDED_XMLTOK_H_

#include <vector>
#include <string>
#include <utility>
#include "Util.h"

namespace qhub {

class XmlTok {
public:
	static char indentChar;
	/*
	 * Getters
	 */
	XmlTok(std::string const& name = Util::emptyString, XmlTok* parent = NULL) throw();
	virtual ~XmlTok() throw();
	bool findChild(std::string const& name) const throw();
	XmlTok* getNextChild() const throw();
	XmlTok* getParent() const throw();
	std::string const& getAttr(std::string const& name) const throw();
	// NOTE: see setData
	std::string toString(int indent = 0) const throw();
	// NOTE: see setData
	const std::string& getData() const throw();
	/*
	 * Setters
	 */
	XmlTok* addChild(std::string const& name) throw();
	void setAttr(std::string const& name, std::string const& attr) throw();
	// NOTE: DATA is not get'able if you have children as well!
	void setData(std::string const& data) throw();
	/*
	 * Load/Save
	 */
	bool load(std::string const& filename) throw();
	bool save(std::string const& filename) const throw();
	/*
	 * Iterators
	 */
	typedef std::vector<XmlTok*>::iterator iterator;
	iterator begin() { return children.begin(); };
	iterator end() { return children.end(); };

	void clear() throw();
private:
	std::string data;
	XmlTok* parent;
	std::string name;
	typedef std::vector<XmlTok*> Children;
	Children children;
	mutable Children::const_iterator found;
	StringMap attributes;
};

} //namespace qhub

#endif //_INCLUDED_XMLTOK_H_
