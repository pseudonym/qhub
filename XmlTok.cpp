// vim:ts=4:sw=4:noet
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "XmlTok.h"
#include <stdio.h>

#if defined(HAVE_EXPAT_H)
# include <expat.h>
#elif defined(HAVE_XMLTOK_XMLPARSE_H)
# include <xmltok/xmlparse.h>
#else
# error expat or xmlparse missing
#endif

using namespace std;
using namespace qhub;

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
	if(sizeof(XML_Char) != 1) {
		fprintf(stderr, "XmlTok::XmlTok UTF8 compilation.. not supported FIXME\n");
		exit(1);
	}
}

XmlTok::~XmlTok() throw()
{
	for(Children::iterator i = children.begin(); i != children.end(); ++i)
		delete *i;
}

bool XmlTok::findChild(string const& n) throw()
{
	for(found = children.begin(); found != children.end(); ++found) {
		if((*found)->name == n)
			return true;
	}
	return false;
}

XmlTok* XmlTok::getNextChild() throw()
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

XmlTok* XmlTok::getParent() throw()
{
	return parent;
}

string const& XmlTok::getAttr(string const& n) const throw()
{
	for(Attributes::const_iterator i = attributes.begin(); i != attributes.end(); ++i) {
		if(i->first == n)
			return i->second;
	}
	return empty;
}

string XmlTok::toString(int indent) const throw()
{
	string ret;
	for(Children::const_iterator i = children.begin(); i != children.end(); ++i) {
		XmlTok* p = *i;
		ret += string(indent, indentChar) + "<" + p->name;
		for(Attributes::const_iterator j = p->attributes.begin(); j != p->attributes.end(); ++j)
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

string XmlTok::getData() const throw()
{
	if(children.empty())
		return data;
	return "";
}

XmlTok* XmlTok::addChild(string const& n) throw()
{
	XmlTok* tmp = new XmlTok(n, this);
	children.push_back(tmp);
	//fprintf(stderr, "setting parent %p %p to %s\n", children.back().getParent(), this, name.c_str());
	return children.back();
}

void XmlTok::setAttr(string const& n, string const& attr) throw()
{
	attributes.push_back(make_pair(n, attr));
}

void XmlTok::setData(string const& d) throw()
{
	data = d;
}

bool XmlTok::load(string const& filename) throw()
{
	FILE* fp = fopen(filename.c_str(), "r");
	if(!fp) {
		perror("XmlTok::load: fopen");
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
			fprintf(stderr, "XmlTok::load: Read error\n");
			fclose(fp);
			return false;
		}
		done = len < sizeof(buf);
		if(!XML_Parse(parser, buf, len, done)) {
			fprintf(stderr, "XmlTok::load: %s at line %d\n",
					XML_ErrorString(XML_GetErrorCode(parser)),
					XML_GetCurrentLineNumber(parser));
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
		perror("XmlTok::save: fopen");
		return false;
	}
	
	string total = toString();
	size_t left = total.length();
	do {
		left -= fwrite(total.c_str(), 1, left, fp);
		if(ferror(fp)) {
			fprintf(stderr, "XmlTok::save: Write error\n");
			fclose(fp);
			return false;
		}
	} while(left);
	fclose(fp);
	return true;
}

/*
int main()
{
	XmlTok root;
	XmlTok* p = &root;

	p = p->addChild("qhub");
	p->setAttr("enabled", "true");
	p = p->addChild("server");
	p->setAttr("address", "0.0.0.0");
	p->setAttr("port", "9001");
	p = p->getParent();
	p = p->addChild("motd");
	p->setData("Testhub bla bla");
	
	fprintf(stderr, "%s\n", root.toString().c_str());
}
*/

/*
int main()
{
	XmlTok root;
	root.load("xml");
	fprintf(stderr, "%s\n", root.toString().c_str());

	XmlTok* p = &root;
	if(p->findChild("xml")) {
		p = p->getNextChild();
		if(p->findChild("qhub")) {
			p = p->getNextChild();
			
			// single item
			if(p->findChild("server")) {
				p = p->getNextChild();
				fprintf(stderr, "serveraddress = %s\n", p->getAttr("address").c_str());
				fprintf(stderr, "serverport = %s\n", p->getAttr("port").c_str());
				p = p->getParent();
			}

			// list
			XmlTok* tmp;
			p->findChild("motd");
			while((tmp = p->getNextChild()))
				fprintf(stderr, "motddata = %s\n", tmp->getData().c_str());

			p = p->getParent();
		}
		p = p->getParent();
	}		
	return 0;
}
*/
