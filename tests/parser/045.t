// forward declaration of prototype
int main();

// another one wich is legal
int main();

// actual definition
int main()
{
  return 0;
}

// another prototype is still legal
int main();

// but this yields in redefinition
int main()
{

}