/* aliases for the exported symbols */
#define foo	foo1_LTX_foo
#define bar	foo1_LTX_bar

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


