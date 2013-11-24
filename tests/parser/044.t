const struct s { int mem; } cs;

// the object ncs is modifiable
struct s ncs; 

typedef int A[2][3];

// array of array of const int
const A a;

int *pi;

const int *pci;

int main()
{
// valid
  ncs = cs; 

// violates modifiable lvalue constraint for =
  cs = ncs; 

// valid
  pi = &ncs.mem;

// violates type constraints for =
  pi = &cs.mem; 

// valid
  pci = &cs.mem; 

// invalid: a[0] has type ‘‘const int *’’
  pi = a[0]; 
}