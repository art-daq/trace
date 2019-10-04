 // This file (TLOG_manipulators.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Jul 18, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: TLOG_manipulators.cc,v $
 // rev="$Revision: 1.1 $$Date: 2019/07/18 15:28:45 $";
/*
rm -fr Linux*; \
make OUT=$PWD\
 XTRA_CFLAGS='-std=c11 -O0'\
 XTRA_CXXFLAGS='-std=c++11 -O0 -DTRACE_STREAMER_MSGMAX=45' 32ON64=1\
 src_utility src_example_user script modules 2>&1|tee make.out|egrep -iB4 'error|warn'

export TRACE_SHOW=%H%n\ %m; treset; tmodeS 0; toffMg 3-63;\
TLOG_manipulators; Linux+3.10-2.17/bin/TLOG_manipulators; \
trace_cntl show;   Linux+3.10-2.17/bin/trace_cntl show

export TRACE_SHOW=%H%n\ %f; treset; tmodeS 0; toffMg 3-63;\
TLOG_manipulators; Linux+3.10-2.17/bin/TLOG_manipulators; \
trace_cntl show;   Linux+3.10-2.17/bin/trace_cntl show

TRACE_SHOW=%f Linux+3.10-2.17/bin/trace_cntl show | while read ln;do echo "$ln" | wc -c;done
 */
#include <stdio.h>		// printf
//#define TRACE_STREAMER_DEBUG 1
#include "TRACE/trace.h"

int
main(  int	argc    __attribute__((__unused__))
     , char	*argv[] __attribute__((__unused__)) )
{
	for (unsigned ii=0; ii<2; ++ii)
		TLOG(TLVL_DEBUG) << "ii+10=" << ii+10 << " "
						<< std::fixed << std::setprecision(1) << 123.456;
	for (unsigned ii=0; ii<2; ++ii) {
		TLOG(TLVL_DEBUG) << "ii+10=" << ii+10 << std::hex << " " << ii+10 << " " << 789.012 << " " << std::setprecision(1) << 123.456;
		TLOG(TLVL_TRACE) << "ii+10=" << ii+10 << std::hex << " " << ii+10 << " "
						 << 789.012 << " " << std::setprecision(2) << 123.456;
	}
	const unsigned char              two=2;
	const int                        thr=3;
	const short                      fou=4;
	const long                       fiv=5;
	const short unsigned             six=6;
	const unsigned                   sev=7;
	const long unsigned              eig=8;
	const long long unsigned         nin=9;
	const double                     ten=10;
	const float                      ele=11;
	const bool                       twe=12;
#  if defined(__cplusplus) && (__cplusplus >= 201103L)
	const std::atomic<int>           tht(13);
    std::atomic<unsigned long> const fot(14);
    std::atomic<short int> const     fit(15);
    std::atomic<bool>          const sit(16);
	std::unique_ptr<int> const       sEt(new int(17));
#  endif
	void *const                      eit((void*)18);
	const char *space="                    ";
	TLOG(2) << '1' <<" " << two <<" " << thr <<" " << fou <<" " << fiv <<" "
			<< six <<" " << sev <<" " << eig <<" " << nin <<" " << ten <<" " << ele;
	for (unsigned ii=0; ii<strlen(space); ii+=(ii+1))
		TLOG(2) << ten <<" " << ele <<" " << twe <<&space[strlen(space)-1-ii]
#  if defined(__cplusplus) && (__cplusplus >= 201103L)
				<< tht <<" " << fot <<" " << fit <<" " << sit <<" " << sEt <<" "
#  endif
				<< (void*)0x12345 <<" " << eit <<" " << 99;
	TLOG(3) << (void*)0x12345678 <<" " << eit <<" sizeof(trace_ptr_t)=" << sizeof(trace_ptr_t)
			<<" " << (long double)3.0                      <<" " << 3
			<<" " << reinterpret_cast<const void*>(0x1234) <<" " << reinterpret_cast<const void*>(0x5678)
			<<" " << reinterpret_cast<long* const>(0x1234) <<" " << reinterpret_cast<long* const>(0x5678)
		;
	return (0);
}   // main
