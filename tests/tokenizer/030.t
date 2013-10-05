"a//b"         // four-character string literal
#include "//e" 
// */          // comment, not syntax error
f = g/**//h;   // equivalent to f = g / h;
//\
i();           // part of a two-line comment
/\
/ j();         // part of a two-line comment
#define glue(x,y) x##y
glue(/,/) k(); // syntax error, not comment
/*//*/ l();    // equivalent to l();
m = n//**/o
+ p;           // equivalent to m = n + p;ivalent to m = n + p;