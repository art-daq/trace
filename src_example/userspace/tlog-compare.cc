 // This file (tlog-compare.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Sep 24, 2020. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";

#define USAGE "\
  usage: %s [test-msk [modes_msk]]\n\
", basename(argv[0])

#include <libgen.h>				// basename
#include <stdio.h>				// printf
#include <getopt.h>	// this does not appear to be posix, but eliminates gcc v10 warnings
#include <unistd.h>	// getopt
#include <TRACE/trace.h>		// TLOG

#define DFLT_TEST_COMPARE_ITERS 500000

static uint64_t gettimeofday_us() {
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return (uint64_t)(tv.tv_sec*1000000+tv.tv_usec);
}


int main( int argc, char *argv[] ) {
	char      buffer[200];
	uint64_t  mark;
	uint32_t  delta;
	unsigned  loops=DFLT_TEST_COMPARE_ITERS;
	unsigned  test_mask=0x1ff; /* all tests */
	unsigned  modes_msk=0xf; /* all mode combinations */
	int       fd;
	int       opt, args;                     /* for how I use getopt */
	int       opt_loops=-1;

	while ((opt=getopt(argc,argv,"?hl:")) != -1){
		switch (opt){
			/* '?' is also what you get w/ "invalid option -- -" */
		case '?': case 'h': printf( USAGE ); exit (0); break;
		case 'l': opt_loops = (int)strtoul(optarg, NULL, 0); break;
		}
	}
	args = argc - optind;

	if (opt_loops > -1) loops=opt_loops;
	opt_loops = loops;		/* save */

	fd = open( "/dev/null", O_WRONLY );
	dup2( fd, 1 );   /* redirect stdout to /dev/null */
	setlocale(LC_NUMERIC,"en_US");  /* make ' printf flag work -- setting LC_NUMERIC in env does not seem to work */

	TRACE_CNTL("mode",3);
	traceControl_rwp->mode.bits.M = 1;		   // NOTE: TRACE_CNTL("modeM",1) hardwired to NOT enable when not mapped!

#	define STRT_PRN( fmt2args, a1, a2 ) sprintf(buffer,fmt2args,a1,a2);fprintf(stderr,"%-47s",buffer);fflush(stderr)
	// ELF 6/6/18: GCC v6_3_0 does not like %', removing the '...
#	define END_FMT  "%10u us, %6.4f us/TLOG, %8.3f Mtlogs/s\n",delta,(double)delta/loops,(double)loops/delta
#   define CONTINUE fprintf(stderr,"Continuing.\n");continue
	if (args >= 1) test_mask=(unsigned)strtoul(argv[optind],NULL,0);
	if (args >= 2) modes_msk=(unsigned)strtoul(argv[optind+1],NULL,0);
	for (int jj=0; jj<4; ++jj) {
		unsigned tstmod=(1U<<jj)&modes_msk;
		switch ((1U<<jj)&modes_msk) {
		case 1: { TRACE_CNTL("lvlclrM",1LL<<TLVL_INFO); TRACE_CNTL("lvlclrS",1LL<<TLVL_INFO);
				loops = (unsigned)opt_loops*5;
				fprintf(stderr,"0x1 M0S0 - Testing with M and S lvl disabled. loops=%u\n", loops );
		}		break;
		case 2: { TRACE_CNTL("lvlsetM",1LL<<TLVL_INFO); TRACE_CNTL("lvlclrS",1LL<<TLVL_INFO);
				loops = (unsigned)opt_loops*2;
				fprintf(stderr,"0x2 M1S0 - Testing with S lvl disabled (mem only). loops=%u\n", loops ); break;
		}		break;
		case 4: { TRACE_CNTL("lvlsetM",1LL<<TLVL_INFO); TRACE_CNTL("lvlsetS",1LL<<TLVL_INFO);
				loops = (unsigned)opt_loops;
				fprintf(stderr,"0x4 M1S1 - Testing with M and S lvl enabled (stdout>/dev/null). loops=%u\n", loops ); break;
		}		break;
		case 8: { TRACE_CNTL("lvlclrM",1LL<<TLVL_INFO); TRACE_CNTL("lvlsetS",1LL<<TLVL_INFO);
				loops = (unsigned)opt_loops;
				fprintf(stderr,"0x8 M0S1 - Testing with just S lvl enabled. Unusual (freeze). loops=%u\n", loops ); break;
		}		break;
		case 0: continue;
		}

		if (1 & test_mask) {
			STRT_PRN(" 0x001 -%s const short msg %s","",(tstmod&0xc)?"(NO snprintf)":"");
			TRACE_CNTL("reset"); mark = gettimeofday_us();
			for (unsigned uu=0; uu<loops; ++uu)
				TLOG(TLVL_INFO) << "any msg";
			delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
		}

		if (2 & test_mask) {
			STRT_PRN(" 0x002 - 1 arg%s%s","","");
			TRACE_CNTL("reset"); mark = gettimeofday_us();
			for (unsigned uu=0; uu<loops; ++uu)
				TLOG(TLVL_INFO) << "this is one small param: " << 12345678;
			delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
		}

		if (4 & test_mask) {
			STRT_PRN(" 0x004 - 2 args%s%s","","");
			TRACE_CNTL("reset"); mark = gettimeofday_us();
			for (unsigned uu=0; uu<loops; ++uu)
				TLOG(TLVL_INFO) << "this is 2 params: " << 12345678 << " " << uu;
			delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
		}

		if (8 & test_mask) {
			STRT_PRN(" 0x008 - 8 args (7 ints, 1 float)%s%s","","");
			TRACE_CNTL("reset"); mark = gettimeofday_us();
			for (unsigned uu=0; uu<loops; ++uu)
				TLOG(TLVL_INFO) << "this is 8 params: " << 12345678 << " " << uu
								<< " " << uu*2 << " " << uu+6 << " " << 12345679
								<< " " << uu   << " " << uu-7 << " " << (float)uu*1.5;
			delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
		}

		if (0x10 & test_mask) {
			STRT_PRN(" 0x010 - 8 args (1 ints, 7 float)%s%s","","");
			TRACE_CNTL("reset"); mark = gettimeofday_us();
			for (unsigned uu=0; uu<loops; ++uu)
				TLOG(TLVL_INFO) << "this is 8 params: " << 12345678 << " " << (float)uu
								<< " " << (float)uu*2.5 << " " << (float)uu+3.14 << " " << (float)12345679
								<< " " << (float)uu/.25 << " " << (float)uu-2*3.14 << " " << (float)uu*1.5;
			delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
		}

		if (0x20 & test_mask) {
			STRT_PRN(" 0x020 - snprintf of same 8 args%s%s","","");
			TRACE_CNTL("reset"); mark = gettimeofday_us();
			for (unsigned uu=0; uu<loops; ++uu) {
				snprintf( buffer, sizeof(buffer)
				         , "this is 8 params: %u %g %g %g %g %g %g %g"
				         , 12345678, (float)uu, (float)uu*2.5, (float)uu+3.14
				         , (float)12345679, (float)uu/.25, (float)uu-2*3.14, (float)uu*1.5
				         );
				TLOG(TLVL_INFO) << buffer;
			} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
		}

		if (0x40 & test_mask) {
			STRT_PRN(" 0x040 -%s const short msg %s",(1&test_mask)?" (repeat)":"",(tstmod&0xc)?"(NO snprintf)":"");
			TRACE_CNTL("reset"); mark = gettimeofday_us();
			for (unsigned uu=0; uu<loops; ++uu)
				TLOG(TLVL_INFO) << "any msg";
			delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
		}

		if (0x80 & test_mask) {
			STRT_PRN(" 0x080 - 2 args%s%s traceTID=-1","","");
			TRACE_CNTL("reset"); mark = gettimeofday_us();
			for (unsigned uu=0; uu<loops; ++uu) {
				TLOG(TLVL_INFO) << "this is 2 params: " << 12345678 << " " << uu;
				traceTID=-1;
			} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
		}

		if (0x100 & test_mask) {
#		   undef TLOG
#		   include <iostream>
#		   define TLOG(...) if(0)std::cout
#		   ifndef __OPTIMIZE__
			STRT_PRN(" 0x100 - 2 args%s%s "," - NoTLOG - ","NOT Optimized.");
#		   else
			STRT_PRN(" 0x100 - 2 args%s%s "," - NoTLOG - ","OPTIMIZED out.");
#		   endif // __OPTIMIZE__
			TRACE_CNTL("reset"); mark = gettimeofday_us();
			for (unsigned uu=0; uu<loops; ++uu) {
				TLOG(TLVL_INFO) << "this is 2 params: " << 12345678 << " " << uu;
			} delta=(uint32_t)(gettimeofday_us()-mark); fprintf(stderr,END_FMT);
		}
	} // for (jj<4)
	return (0);
}   // main
