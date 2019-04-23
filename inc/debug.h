// DEBUG PRINTING


// If you want the program to print out debugging info
// put this define in your code before this header is included
// #define DEBUG 1
#include <stdio.h>
#include <time.h>
// This is gross, but it makes it so the compiler always checks the syntax even when not defined
// This makes it so you can't break debug prints while they're turned off
#define debug_print(...) do{ if(DEBUG){printf(__VA_ARGS__);}}while(0)

inline void debug_print_time(void)
{
	if(DEBUG)
	{
		struct timespec debug_time;
		clock_gettime(CLOCK_MONOTONIC, &debug_time);
		printf("%ld.%ld", debug_time.tv_sec, debug_time.tv_nsec);
	}
}