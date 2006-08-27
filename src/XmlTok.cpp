// vim:ts=4:sw=4:noet
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "XmlTok.h"
#include "Util.h"
#include "Logs.h"
#include <cstdio>
#include <cerrno>

#include <boost/static_assert.hpp>
#include <boost/array.hpp>

#if defined(HAVE_EXPAT_H)
# include <expat.h>
#elif defined(HAVE_XMLTOK_XMLPARSE_H)
# include <xmltok/xmlparse.h>
#else
# error expat or xmlparse missing
#endif

using namespace std;
using namespace qhub;

BOOST_STATIC_ASSERT(sizeof(XML_Char) == 1);

char XmlTok::indentChar = '\t';

XmlTok::XmlTok(string const& n, XmlTok* p) throw()
		: parent(p), name(n)
{
}

XmlTok::XmlTok(istream& is) throw(io_error)
{
	XML_Parser parser = XML_ParserCreate(NULL);
	XmlTok* p = this;
	boost::array<XML_Char, 1024> tmp;

	XML_SetUserData(parser, &p);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, dataElement);

	streamsize n;
	do {
		n = is.readsome(tmp.c_array(), tmp.size());
		Logs::stat << "Read XML: \"" << string(tmp.data(), n) << "\", length " << n << endl;
		if(!XML_Parse(parser, tmp.data(), n, n < tmp.size())) {
			format fmt("XML loading failed: %s at line %d\n");
			fmt % XML_ErrorString(XML_GetErrorCode(parser)) % XML_GetCurrentLineNumber(parser);
			XML_ParserFree(parser);
			Logs::err << fmt;
			throw io_error(fmt.str());
		}
	} while(n == tmp.size());

	XML_ParserFree(parser);
}

XmlTok::~XmlTok() throw()
{
	for(Children::iterator i = children.begin(); i != children.end(); ++i)
		delete *i;
}

bool XmlTok::findChild(string const& n) const throw()
{
	for(found = children.begin(); found != children.end(); ++found) {
		if((*found)->name == n)
			return true;
	}
	return false;
}

XmlTok* XmlTok::getNextChild() const throw()
{
	if(found == children.end())
		return NULL;
	XmlTok* p = *found;
	string tmpname = p->name;
	for(++found; found != children.end(); ++found) {
		if((*found)->name == tmpname)
			break;
	}
	return p;
}

XmlTok* XmlTok::getParent() const throw()
{
	return parent;
}

string const& XmlTok::getAttr(string const& n) const throw()
{
	if(attributes.count(n))
		return attributes.find(n)->second;
	return Util::emptyString;
}

void XmlTok::save(ostream& out, int indent) const throw()
{
	out << string(indent, indentChar) << "<" << getName();

	for(StringMap::const_iterator i = attributes.begin(); i != attributes.end(); ++i)
		out << ' ' << i->first << "=\"" << i->second << '"';

	if(children.empty()) {
		if(getData().empty()) {
			out << "/>" << endl;
		} else {
			out << ">" << getData() << "</" << getName() << ">" << endl;
		}
	} else {
		out << ">\n";
		for(Children::const_iterator i = children.begin(); i != children.end(); ++i)
			(*i)->save(out, indent + 1);
		out << string(indent, indentChar) << "</" << getName() << '>' << endl;
	}
}

const string& XmlTok::getData() const throw()
{
	if(children.empty())
		return data;
	return Util::emptyString;
}

XmlTok* XmlTok::addChild(string const& n) throw()
{
	XmlTok* tmp = new XmlTok(n, this);
	children.push_back(tmp);
	return children.back();
}

void XmlTok::setAttr(string const& n, string const& attr) throw()
{
	attributes.insert(make_pair(n, attr));
}

void XmlTok::setData(string const& d) throw()
{
	data = d;
}

void XmlTok::save(ostream& out) const throw(io_error)
{
	save(out, 0);
	if(!out)
		throw io_error("Failed saving XML file");
}

void XmlTok::clear() throw()
{
	attributes.clear();
	for(Children::iterator i = children.begin(); i != children.end(); ++i)
		delete *i;
	children.clear();
}

void XmlTok::startElement(void *userData, char const* name, char const** atts)
{
	XmlTok** p = (XmlTok**)userData;
	XmlTok* x = *p;

	// if root element, set name, don't add child
	if(!x->getParent() && x->getName().empty()) {
		Logs::stat << "setting name to " << name << endl;
		x->name = name;
	} else {
		Logs::stat << "adding child " << name << " to node " << x->getName() << endl;
		x = x->addChild(name);
	}

	while(atts[0]) {
		Logs::stat << "setting attrs in " << x->getName() << ": " << atts[0] << '=' << atts[1] << endl;
		x->setAttr(atts[0], atts[1]);
		atts += 2;
	}
	*p = x;
}

void XmlTok::endElement(void *userData, char const* name)
{
	XmlTok** p = (XmlTok**)userData;
	XmlTok* x = *p;
	x = x->getParent();
	*p = x;
}

void XmlTok::dataElement(void *userData, XML_Char const* data, int length)
{
	XmlTok** p = (XmlTok**)userData;
	XmlTok* x = *p;
	Logs::stat << "setting data of " << x->getName() << " to " << string(data, length) << endl;
	x->setData(string(data, length));
}
