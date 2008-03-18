// vim:ts=4:sw=4:noet
#ifndef QHUB_XMLTOK_H
#define QHUB_XMLTOK_H

#include <vector>
#include <string>
#include <utility>
#include "Util.h"
#include "error.h"

namespace qhub {

class XmlTok {
public:
	static char indentChar;
	/*
	 * Getters
	 */
	XmlTok(std::istream& in) throw(io_error);
	XmlTok(std::string const& name, XmlTok* parent = NULL) throw();
	virtual ~XmlTok() throw();
	bool findChild(std::string const& name) const throw();
	XmlTok* getNextChild() const throw();
	XmlTok* getParent() const throw();
	std::string const& getName() const throw() { return name; }
	std::string const& getAttr(std::string const& name) const throw();
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
	void save(std::ostream& out) const throw(io_error);
	/*
	 * Iterators
	 */
	typedef std::vector<XmlTok*>::iterator iterator;
	iterator begin() { return children.begin(); };
	iterator end() { return children.end(); };

	void clear() throw();
private:
	// NOTE: see setData
	void save(std::ostream& out, int indent) const throw();

	std::string data;
	XmlTok* parent;
	std::string name;
	typedef std::vector<XmlTok*> Children;
	Children children;
	mutable Children::const_iterator found;
	StringMap attributes;

	static void startElement(void *userData, char const* name, char const** atts);
	static void endElement(void *userData, char const* name);
	static void dataElement(void *userData, char const* data, int length);
};

} //namespace qhub

#endif //QHUB_XMLTOK_H
