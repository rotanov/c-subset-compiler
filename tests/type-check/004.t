const struct s { int mem; };

// the object ncs is modifiable
struct s ncs; 

int *pi;

int main()
{
  // valid
  pi = &ncs.mem;
}