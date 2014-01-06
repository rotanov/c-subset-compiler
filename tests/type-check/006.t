const struct s { int mem; } cs;
const int *pci;

int main()
{
  // valid
  pci = &cs.mem; 
}