const struct s { int mem; } cs;

int *pi;

const int *pci;

int main()
{
  // violates type constraints for =
  pi = &cs.mem; 
}