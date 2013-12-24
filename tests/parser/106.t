int f()
{
  return 2;
}

struct foo
{
int c;
};

int main()
{
int a = 1;
a = a + a / a * a - a;
int b[2][8];
b[0][1] = a;

// f = 2; // !!!
foo bar;
bar.c;
foo* baf;
baf->c;
}