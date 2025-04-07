#if defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
	int main(int, char*[]) {return 0;}
#else
	#error Not x86-32.
#endif
