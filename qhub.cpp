#include "config.h"
#include "qhub.h"
#include "MyTime.h"

#include <stdio.h>

#include "DNSUser.h"
#include "Hub.h"


#ifdef HAVE_XERCESC_DOM_DOM_HPP
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <iostream>
#endif

#include <string>

using namespace std;
using namespace xercesc;

extern "C" {
#include <oop.h>
#include <oop-read.h>

#include <adns.h>
#include <oop-adns.h>
#ifdef HAVE_LIBOOP_EVENT 
#include <oop-event.h>
#endif
}

#ifdef HAVE_LIBOOP_EVENT
#include <event.h>
#endif

using namespace qhub;

static oop_adapter_adns *adns;

static void *on_lookup(oop_adapter_adns *adns,adns_answer *reply,void *data)
{
	fprintf(stdout, "%s =>",reply->owner);
	fflush(stdout);

	DNSUser* d = (DNSUser*) data;

	if (adns_s_ok != reply->status) {
		printf(" error: %s\n",adns_strerror(reply->status));
	} else {
		if (NULL != reply->cname) {
			printf(" (%s)",reply->cname);
		}
		assert(adns_r_a == reply->type);
		for (int i = 0; i < reply->nrrs; ++i) {
			printf(" %d: %s",i, inet_ntoa(reply->rrs.inaddr[i]));
		}
		printf("\n");
	}

	return OOP_CONTINUE;
}

void *fd_demux(oop_source *src, int fd, oop_event ev, void* usr)
{
	Socket* s = (Socket*) usr;

	switch(ev){
		case OOP_READ:
			s->on_read();
			break;
		case OOP_WRITE:
			s->on_write();
			break;
	}

	return OOP_CONTINUE;
}

static oop_source* src;

void qhub::enable(int fd, oop_event ev, Socket* s)
{
	fprintf(stderr, "Enabling fd %d %d\n", fd, ev);
	src->on_fd(src, fd, ev, fd_demux, s);
}

void qhub::cancel(int fd, oop_event ev)
{
	fprintf(stderr, "Cancellng fd %d %d\n", fd, ev);
	src->cancel_fd(src, fd, ev);
}

void qhub::lookup(const char* hostname, DNSUser* d){
	oop_adns_query * qadns = oop_adns_submit(adns,NULL,hostname,adns_r_a,adns_qf_owner,on_lookup,d);
}

int main()
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
#ifdef LINUX
	signal(SIGCLD, SIG_IGN);
#else
	signal(SIGCHLD, SIG_IGN);
#endif

#ifdef HAVE_LIBOOP_EVENT
	event_init();
#endif

#ifndef HAVE_LIBOOP_EVENT
	oop_source_sys* system;

	if((system=oop_sys_new()) == NULL){
		fprintf(stderr, "Malloc failure.\n");
		exit(1);
	}
	src = oop_sys_source(system);
	fprintf(stderr, "Using liboop system event source: select() will be used.\n");
#else
	src = oop_event_new();
	fprintf(stderr, "Using libevent source adapter\n", src);
#endif
	//Set up ADNS
	adns = oop_adns_new(src,(adns_initflags)0,NULL);

#ifdef HAVE_XERCESC_DOM_DOM_HPP
	fprintf(stderr, "Using Xerces XML library.\n");

	try {
		XMLPlatformUtils::Initialize();
	}
	catch (const XMLException& toCatch) {
		char* message = XMLString::transcode(toCatch.getMessage());
		cout << "Error during initialization! :\n"
		<< message << "\n";
		XMLString::release(&message);
		return 1;
	}

	XercesDOMParser* parser = new XercesDOMParser();	
	if(parser){
		ErrorHandler* errHandler = (ErrorHandler*) new HandlerBase();
		parser->setErrorHandler(errHandler);
		char* xmlFile = "config.xml";

		try {
			parser->parse(xmlFile);
		}
		catch (const XMLException& toCatch) {
			char* message = XMLString::transcode(toCatch.getMessage());
			cout << "Exception message is: \n"
			<< message << "\n";
			XMLString::release(&message);
			return -1;
		}
		catch (const DOMException& toCatch) {
			char* message = XMLString::transcode(toCatch.msg);
			cout << "Exception message is: \n"
			<< message << "\n";
			XMLString::release(&message);
			return -1;
		}
		catch (...) {
			cout << "Unexpected Exception \n" ;
			return -1;
		}
		
		DOMNode *pDoc = parser->getDocument();
		pDoc = pDoc->getFirstChild();
		if(pDoc->getNextSibling() != NULL){
			pDoc = pDoc->getNextSibling();
		}
		DOMNode* c = pDoc->getFirstChild();
		while(c){
			//this should be a hub
			if(c->getFirstChild() != NULL){
				DOMNode* b = c->getFirstChild();
				Hub* tmp = new Hub();
				while(b){
					if(strcmp(XMLString::transcode(b->getNodeName()), "name") == 0 && b->getFirstChild() != NULL){
						cout << "Hubname: " << XMLString::transcode(b->getFirstChild()->getNodeValue()) << endl;
						tmp->setHubName(string(XMLString::transcode(b->getFirstChild()->getNodeValue())));
					} else if(strcmp(XMLString::transcode(b->getNodeName()), "port") == 0 && b->getFirstChild() != NULL){
						cout << "\tADC port: " << XMLString::transcode(b->getFirstChild()->getNodeValue()) << endl;
						int port = atoi(XMLString::transcode(b->getFirstChild()->getNodeValue()));
						if(port > 0 && port<65536){
							tmp->openADCPort(port);
						}
					} else if(strcmp(XMLString::transcode(b->getNodeName()), "interport") == 0 && b->getFirstChild() != NULL){
						cout << "\tInter-hub port: " << XMLString::transcode(b->getFirstChild()->getNodeValue()) << endl;
						int port = atoi(XMLString::transcode(b->getFirstChild()->getNodeValue()));
						if(port > 0 && port<65536){
							tmp->openInterPort(port);
						}
					} else if(strcmp(XMLString::transcode(b->getNodeName()), "interconnect") == 0 && b->getFirstChild() != NULL){
						cout << "\tInter-connecting to: " << XMLString::transcode(b->getFirstChild()->getNodeName()) << endl;
							DOMNode*a = b->getFirstChild();
							string host, password;
							int port;
							while(a){
								if(strcmp(XMLString::transcode(a->getNodeName()), "host") == 0 && a->getFirstChild() != NULL){
									host = XMLString::transcode(a->getFirstChild()->getNodeValue());
								} else if(strcmp(XMLString::transcode(a->getNodeName()), "port") == 0 && a->getFirstChild() != NULL){
									port = atoi(XMLString::transcode(a->getFirstChild()->getNodeValue()));
								} else if(strcmp(XMLString::transcode(a->getNodeName()), "password") == 0 && a->getFirstChild() != NULL){
									password = XMLString::transcode(a->getFirstChild()->getNodeValue());
								}
								a = a->getNextSibling();
							}
							cout << "\tConnecting to " << host << ":" << port << " pass: " << password << endl;
					}

					b = b->getNextSibling();
				}
			}
			c = c->getNextSibling();
		}

		delete parser;
		delete errHandler;

	}

	XMLPlatformUtils::Terminate();
#else
	fprintf(stderr, "Warning: Xerces XML parser not used, no XML config file will be loaded.\n");
	Hub* tmp = new Hub();
	tmp->openADCPort(9001);
#endif

#ifndef HAVE_LIBOOP_EVENT
	oop_sys_run(system);
#else
	event_dispatch();
#endif
	return 0;
}
