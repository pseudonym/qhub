/* aliases for the exported symbols */
#define foo	foo1_LTX_foo
#define bar	foo1_LTX_bar
#define login	foo1_LTX_login

#include "ADC.h"
#include "ADCInf.h"

extern "C" {

/* a global variable definition */
int bar = 1;

/* a private function */
int _foo1_helper() {
  return bar;
}

/* an exported function */
int foo() {
  return _foo1_helper();
}

int login(void* c) {
	qhub::ADC& client = *(qhub::ADC*)c;
	qhub::ADCInf& attr = *client.getAttr();
	if(attr.getInf("NI").find("sed") != string::npos) {
		client.doAskPassword("hoi");
	} else if(attr.getInf("NI").find("sandos") != string::npos) {
		client.doAskPassword("majs");
	}
	return 0;
}

int authenticated(void* c) {
	qhub::ADC& client = *(qhub::ADC*)c;
	qhub::ADCInf& attr = *client.getAttr();
	attr.setInf("OP", "1");
	return 0;
}

}
