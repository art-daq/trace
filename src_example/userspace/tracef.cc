/*
 * make XTRA_CXXFLAGS=-std=c++20 tracef
 * OR 
 * g++ -g -Wall -pedantic -O2 -std=c++20 -I../../include -o tracef{,.cc}
 */
#include <format>
#define TRACE_STD_STRING_FORMAT std::format
#include <TRACE/trace.h>

int main()
{
	TRACEF(TLVL_LOG, "This is an int: {}", 5);
	return 0;
}
