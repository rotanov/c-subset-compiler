typedef struct s1 { int x; } t1, *tp1;
typedef struct s2 { int x; } t2, *tp2;

//type t1 and the type pointed to by tp1 are compatible.
// Type t1 is also compatible with type struct s1,
// but not compatible with the types 
// struct s2, t2, the type pointed to by tp2, or int.

// TODO: compatibility test of the above