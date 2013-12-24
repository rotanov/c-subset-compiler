int b[10];

int* f()
{
  return b;
}


int main()
{
  int b;
  b = f()[0];
  f()[0] = 3;
}