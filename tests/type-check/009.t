typedef struct s1 { int x; } t1, *tp1;
typedef struct s2 { int x; } t2, *tp2;

int main()
{
  s1 a;
  s2 b;
  a = b;
}