#if defined(__x86_64__) || defined(_M_X64)
	int main(int, char*[]) {return 0;}
#else
	#error Not x86-64.
#endif
