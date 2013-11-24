// struct forward, ok
struct A;

// another struct forward, ok
struct A;

// wrong: A is still incomplete
// struct A b;

// still wrong, reason is same
// A bb;

// ok, pointers to incomplete types are pretty legal
struct A* c;

// this is also legal
A* noStructKwdPtr;

// actual A definition
struct A
{
    int wahahaha;
};

// forward after A has been defined. still legal
struct A;

// now it's ok, A is complete
struct A b;

// wrong: A already defined
struct A
{
   float wut; 
};

