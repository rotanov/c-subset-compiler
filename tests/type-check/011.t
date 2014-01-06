// All three of the following declarations of
// the signal function specify exactly the
// same type, the first without making use
// of any typedef names.

typedef void fv(int), (*pfv)(int);

void (*signal(int, void (*)(int)))(int);
fv *signal(int, fv *);
pfv signal(int, pfv);

int main()
{
    
}