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

static void startElement(void *userData, char const* name, char const** atts)
{
	XmlTok** p = (XmlTok**)userData;
	XmlTok* x = *p;
	x = x->addChild(name);
	while(atts[0]) {
		x->setAttr(atts[0], atts[1]);
		atts += 2;
	}
	*p = x;
}

static void endElement(void *userData, char const* name)
{
	XmlTok** p = (XmlTok**)userData;
	XmlTok* x = *p;
	x = x->getParent();
	*p = x;
}

static void dataElement(void *userData, XML_Char const* data, int length)
{
	XmlTok** p = (XmlTok**)userData;
	XmlTok* x = *p;
	x->setData(string(data, length));
}

char XmlTok::indentChar = '\t';

XmlTok::XmlTok(string const& n, XmlTok* p) throw()
		: parent(p), name(n)
{
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

string XmlTok::toString(int indent) const throw()
{
	string ret;
	for(Children::const_iterator i = children.begin(); i != children.end(); ++i) {
		XmlTok* p = *i;
		ret += string(indent, indentChar) + "<" + p->name;
		for(StringMap::const_iterator j = p->attributes.begin(); j != p->attributes.end(); ++j)
			ret += ' ' + j->first + "=\"" + j->second + "\"";
		if(p->children.empty()) {
			if(p->data.empty()) {
				ret += "/>\n";
			} else {
				ret += ">" + p->data + "</" + p->name + ">\n";
			}
		} else {
			ret += ">\n";
			ret += p->toString(indent + 1);
			ret += string(indent, indentChar) + "</" + p->name + ">\n";
		}
	}
	return ret;
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

bool XmlTok::load(string const& filename) throw()
{
	FILE* fp = fopen(filename.c_str(), "r");
	if(!fp) {
		Logs::err << "XmlTok::load: fopen: " << Util::errnoToString(errno) << endl;
		return false;
	}

	char buf[BUFSIZ];
	XML_Parser parser = XML_ParserCreate(NULL);
	bool done;
	XmlTok* p = this;

	XML_SetUserData(parser, &p);
	XML_SetElementHandler(parser, startElement, endElement);
	XML_SetCharacterDataHandler(parser, dataElement);

	do {
		size_t len = fread(buf, 1, sizeof(buf), fp);
		if(ferror(fp)) {
			Logs::err << "XmlTok::load: Read error\n";
			fclose(fp);
			return false;
		}
		done = len < sizeof(buf);
		if(!XML_Parse(parser, buf, len, done)) {
			Logs::err << format("XmlTok::load: %s at line %d\n") %
			        XML_ErrorString(XML_GetErrorCode(parser)) %
			        XML_GetCurrentLineNumber(parser);
			fclose(fp);
			return false;
		}
	} while(!done);
	XML_ParserFree(parser);
	fclose(fp);
	return true;
}

bool XmlTok::save(string const& filename) const throw()
{
	FILE* fp = fopen(filename.c_str(), "w");
	if(!fp) {
		Logs::err << "XmlTok::save: fopen: " << Util::errnoToString(errno) << endl;
		return false;
	}

	string total = toString();
	size_t left = total.length();
	do {
		left -= fwrite(total.c_str(), 1, left, fp);
		if(ferror(fp)) {
			Logs::err << "XmlTok::save: Write error";
			fclose(fp);
			return false;
		}
	} while(left);
	fclose(fp);
	return true;
}

void XmlTok::clear() throw()
{
	attributes.clear();
	for(Children::iterator i = children.begin(); i != children.end(); ++i)
		delete *i;
	children.clear();
}
