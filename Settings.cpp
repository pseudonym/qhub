#include "Settings.h"
#include "Hub.h"

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <iostream>

using namespace xercesc;
using namespace qhub;
using namespace std;

int Settings::readFromXML()
{
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
		char* xmlFile = "Settings.xml";

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
					} else if(strcmp(XMLString::transcode(b->getNodeName()), "maxpacketsize") == 0 && b->getFirstChild() != NULL){
						cout << "\tMaximum packet size: " << XMLString::transcode(b->getFirstChild()->getNodeValue()) << endl;
						int size = atoi(XMLString::transcode(b->getFirstChild()->getNodeValue()));
						if(size > 0){
							tmp->setMaxPacketSize(size);
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
						tmp->openInterConnection(host, port, password);
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
}

