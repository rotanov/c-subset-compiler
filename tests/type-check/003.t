const struct s { int mem; } cs;

// the object ncs is modifiable
struct s ncs; 

int main()
{
  // violates modifiable lvalue constraint for =
  cs = ncs; 
}