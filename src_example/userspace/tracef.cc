/*
 * make XTRA_CXXFLAGS=-std=c++20 tracef
 * OR
 * g++ -g -Wall -pedantic -O2 -std=c++20 -I../../include -o tracef{,.cc}
 */

#if __cplusplus >= 202002L
#	include <format>
#	define TRACE_STD_STRING_FORMAT std::format
#endif
#include <TRACE/trace.h>

int main()
{
#if __cplusplus >= 202002L
	TRACEF(TLVL_LOG, "This is an int: {}", 5);
#endif
	return 0;
}
