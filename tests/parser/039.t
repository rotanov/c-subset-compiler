// all of these are compatible

const int a;

const const const int b;

const int const c;

int const d;

typedef int const IntConst;

typedef const int ConstInt;

typedef int const const IntConstConst;

typedef const const int ConstConstInt;

typedef const int const ConstIntConst;

IntConst e;

typedef IntConst * PtrToIntConst;

typedef const IntConst TwiceConstInt;

typedef const IntConst * PtrToConstIntConst;

typedef IntConst * const ConstPtrToIntConst;

ConstPtrToIntConst f;