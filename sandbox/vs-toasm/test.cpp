// int notMain()
// {
//     return 0;
// }

// int main()
// {

// }

// int notMain2()
// {
//     return 0;
// }

// void VoidNoParams()
// {

// }

// void Void2Params(int a, float b)
// {

// }

//#pragma pack(1)

//

// struct Foo
// {
//     int a;
//     int b;
//     char c;
// };

// int a = 2;
// struct Foo f;
// float b;
// char c = 3;
// int bar[10];
// int* ptr;
// struct Foo* fooPtr;
// char chararrfour[4];
// int sizeCalc[2 * 4 + 1];
// int initialized[] = {1, 2, 3, 10, 16,};
// int arrayOfArray[2][3];
// struct Foo arrayOfStruct[2][3];
// float initFloat = 0.001f;

//////////////////////////////////////////////////

// int foo()
// {
//     return 2;
// }

// char* string = "pe";
// char* anoos = "a";
// int fooInit;
// int singleElementArray[1];

// int main()
// {
//     int bar = foo();
//     char* temp = "abcde";
//     fooInit = foo();
// }

//////////////////////////////////////////////////

// const int a = 2;
// const int c = 3;
// int b[sizeof(a)];
// int e[sizeof(b)];

//


// char* a = "abc";
// char b[3] = {'a', 'b', 'c'};
// char c[3] = "abc";
// char d[] = "abc";
// char e[4] = "abc";
// int asize = sizeof(a);
// int bsize = sizeof(b);
// int csize = sizeof(c);
// int dsize = sizeof(d);
// int esize = sizeof(e);

// char f[1] = "abc";
// int fsize = sizeof(f);

// int aa;
// char ddd[4];

// int main()
// {
//     char* foo = "abcdef";
//     char bar[4] = "cdefghajs";
//     foo = d + 1;
// }


//int a[2][2] = {{1, 2}, {2, 3}};

//char a[10] = "a";

// int a  = { 2};

// struct Foo
// {
//     char c;
//     int a1;
//     char a;
//     int a2;
//     char d;
//     int a3;
// } foo =  {1, 2, 1, 2, 1, 2};

///////////////////////////////////////////////

// struct Foo
// {
//     char c;
//     int a;
// }*** bar;

// int main()
// {
//     int a;
//     //a = 2;
//     (**bar)->a = 2;
// }

////////////////////////////////////////////////////

// int foo()
// {
//     return 2;
// }

// int (**bar)();

// int main()
// {
//     bar = foo;
//     printf("%d", bar());
// }
//////////////////////////////////////////////////////

// typedef int (*fptr)();
// typedef int f();

// void foo(void* bar)
// {
//    ((fptr)bar)();
// }

// void bar(f baz)
// {

// }

// void zak(int baz())
// {
//   baz();
// }

// int foobar()
// {
//     return 2;
// }

// int main()
// {
//     zak(foobar);
// }
//////////////////////////////////////////////////////

// typedef int a();

// a b;

// int main()
// {
//   //a e;
// }

// a e
// {
// return 2;
// }
//////////////////////////////////////////////////////

// struct pes
// {
//     char a;
//     int c;
//     char b;
//     int e, f, g, h;
// } p, f;

// int main()
// {
//   p = f;
// }

//////////////////////////////////////////////////////
// struct Pes
// {
//    int sep;
// } pes;

// int main()
// {
//     int a, b;
//     a = b > 0 ? pes : b;
// }
/////////////

// lol bug
// struct pes
// {
// const int a[10];
// char c;
// };

// struct yes
// {
//     struct pes a;
//     int b;
// }a, b;

// int main()
// {
//     a = b;
// }

////////////////////////////////

// int foo()
// {

// }

// int main()
// {
//     *&foo = &foo;
// }

////////////////////////// WOW padding got zeroed if I {{1}}
// struct Foo
// {
//     char c;
//     int i;
//     int j;
// };

// int main()
// {
//     struct Foo foo = {{1};
// }

// int main()
// {
//     int a = 2;
//     int b = a + 1;
//     int c[2] = { 2, 1};
// }

// struct Bar
// {
//     char a4;
//     char a3[2];
//     int a;
// } baz[2] = {2, 3, 4, 5, 6, 7};

// int barsize = sizeof(struct Bar);
// int bazsize = sizeof (baz);

// char a3[3];

// char p;
// int aaa;
// char p2;

// void bar(char* arg)
// {

// }

// char eeff[5] = {'e', 'e', 'f', 'g', 'g'};

// char* a = "a\0 foo";

// int main()
// {
//     char* b = "b foo";
//     bar("c foo");
// }

// int sizeofa = sizeof(a);

//////////////////////////////////////////////////////////////

// struct Foo
// {
//     int a;
//     char c;    
// };

// int size = sizeof(struct Foo);


////////////////////////////////////
// char c = 1;
// char b = 2;

/////////////////////////////////////////////////////////////////

// char a3[3];

// char p;
// int aaa;
// char p2;

// void bar(char* arg)
// {

// }

// char eeff[5] = {'e', 'e', 'f', 'g', 'g'};

// char* a = "a\0 foo";

// int main()
// {
//     char* b = "b foo";
//     bar("c foo");
// }

// int sizeofa = sizeof(a);
////////////////////////////////////////////////////////////
// int aaaa;
// char* a = "abaaa";
// char* b = "ab";
///////////////////////////////////////////////////////////////
// int a;
// int c;
// int* b = &a + 1;
////////////////////////////////////////////////////////////////
// struct foo{int a;} bar = {{{1}}};
//////////////////////////////////////////////////////////////////////
struct Foo
{
    char c;
    int i;
} foo = {1, 2};

struct Foo fooarr[4] = { {{1}}, {2};