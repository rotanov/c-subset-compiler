int a = 2;

struct A
{
  int a;
  float b;
  char c;
  void* e;

  struct B
  {
    struct A* test; // !!! 
  } eee;
  B t1;
  A t2; // incomplete !
};