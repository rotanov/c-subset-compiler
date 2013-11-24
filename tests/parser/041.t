struct A
{
  int a;
  float b;
  char c;
  void* e;

  struct B
  {
    A* test;
  } eee;
  B t1;
  B* t2; // incomplete !
};