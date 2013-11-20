// should be parsed
// naming parameters results in crash
void (*signal(int, void (*fp)(int)))(int);