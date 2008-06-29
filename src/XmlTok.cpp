// vim:ts=4:sw=4:noet
#include "XmlTok.h"

#include "Util.h"

#include <cerrno>
#include <cstdio>

#include <boost/array.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#if defined(HAVE_EXPAT_H)
# include <expat.h>
#elif defined(HAVE_XMLTOK_XMLPARSE_H)
# include <xmltok/xmlparse.h>
#else
# error expat or xmlparse missing
#endif

using namespace std;
using namespace qhub;

// we can't handle it not being char
BOOST_STATIC_ASSERT(sizeof(XML_Char) == 1);

char XmlTok::indentChar = '\t';

XmlTok::XmlTok(string const& n, XmlTok* p) throw()
		: parent(p), name(n)
{
}

XmlTok::XmlTok(istream& is) throw(io_error)
{
	using namespace boost;

	// when possible, use RAII
	shared_ptr<remove_pointer<XML_Parser>::type> parser(XML_ParserCreate(NULL), XML_ParserFree);
	XmlTok* p = this;
	array<XML_Char, 1024> tmp;

	XML_SetUserData(parser.get(), &p);
	XML_SetElementHandler(parser.get(), startElement, endElement);
	XML_SetCharacterDataHandler(parser.get(), dataElement);

	size_t n;
	do {
		n = is.readsome(tmp.c_array(), tmp.size());
		if(!XML_Parse(parser.get(), tmp.data(), n, n < tmp.size())) {
			boost::format fmt("XML loading failed: %s at line %d\n");
			fmt % XML_ErrorString(XML_GetErrorCode(parser.get())) % XML_GetCurrentLineNumber(parser.get());
			throw io_error(fmt.str());
		}
	} while(n == tmp.size());
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
		x->name = name;
	} else {
		x = x->addChild(name);
	}

	while(atts[0]) {
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
	x->setData(string(data, length));
}
