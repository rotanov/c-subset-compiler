typedef int A[2][3];

// array of array of const int
const A a;

int *pi;

int main()
{
  // invalid: a[0] has type ‘‘const int *’’
  pi = a[0];
}