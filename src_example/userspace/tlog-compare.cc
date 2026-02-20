// This file (tlog-compare.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Sep 24, 2020. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";

struct test_desc_t {
	char const *desc;
	unsigned loop_decimate;
} test_desc[]= {{"const short msg", 1},
				{"1 arg", 1},
				{"2 args", 1},
				{"8 args (7 ints, 1 float)", 1},
				{"8 args (1 ints, 7 float)", 1},
				{"snprintf of same 8 args (stringstream)", 1},
				{"(repeat) const short msg", 1},
				{"2 args traceTID=-1 - first TLOG", 1},
				{"2 args TRACE macro", 1},
				{"8 args (7 ints, 1 float) - TLOG_SCOPED() TLOG_ADD", 1},
				{"8 args (7 ints, 1 float) - TLOG_SCOPED(){TLOG_ADD}", 1},
				{"2 args - NoTLOG - OPTIMIZED out", 1}};
#define USAGE   \
	"\
  usage: %s [opts] [tests_mask [modes_mask]]\n\
modes_mask (combinations of Slow and Fast):\n\
  b0 - M0S0\n\
  b1 - M1S0\n\
  b2 - M1S1\n\
  b3 - M0S1\n\
opts:\n\
  -l<loops>  base loops (default: %u)\n\
  -n  add normalized column\n\
tests_mask:\n", \
		basename(argv[0]), DFLT_TEST_COMPARE_ITERS

#include <libgen.h>  // basename
#include <stdio.h>   // printf
#include <getopt.h>  // this does not appear to be posix, but eliminates gcc v10 warnings
#include <unistd.h>  // getopt
#define TRACE_USE_STATIC_STREAMER 1
#include <TRACE/trace.h>  // TLOG
#include <time.h>         // clock_gettime, struct timespec


#define DFLT_TEST_COMPARE_ITERS 1000000

static uint64_t gettimeofday_ns() /* 2**64 = 10**19.26592 giving 10 digits for the seconds (since 1970) */
{                                 /* 10**10 seconds == 317 years */
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)(ts.tv_sec * 1000000000 + ts.tv_nsec);
}

int count_bits(unsigned input)
{
	int bits= 0;
	for (unsigned bb= 0; bb < (sizeof(input) * 8 /*bitsPerByte*/); ++bb)
		if (input & (1 << bb)) ++bits;
	return bits;
}

int main(int argc, char *argv[])
{
	char buffer[200];
	uint64_t mark;
	uint32_t delta;
	unsigned loops= DFLT_TEST_COMPARE_ITERS;
	unsigned tests_mask= 0xfff; /* all tests */
	unsigned modes_mask= 0xf;   /* all mode combinations */
	int fd;
	int opt, args; /* for how I use getopt */
	int opt_loops= -1;
	bool opt_normalize= false;

	while ((opt= getopt(argc, argv, "?hl:n")) != -1) {
		switch (opt) {
			/* '?' is also what you get w/ "invalid option -- -" */
		case '?':
		case 'h':
			printf(USAGE);
			for (unsigned bb= 0; bb < sizeof(test_desc) / sizeof(test_desc[0]); ++bb) {
				printf(" %cb%d - %s\n", (bb & 3) == 3 ? '_' : ' ', bb, test_desc[bb].desc);
			}
			exit(0);
			break;
		case 'l': opt_loops= (int)strtoul(optarg, NULL, 0); break;
		case 'n': opt_normalize= true;
		}
	}
	args= argc - optind;

	if (opt_loops > -1) loops= opt_loops;
	opt_loops= loops; /* save */

	fd= open("/dev/null", O_WRONLY);
	dup2(fd, 1);                    /* redirect stdout to /dev/null */
	setlocale(LC_NUMERIC, "en_US"); /* make ' printf flag work -- setting LC_NUMERIC in env does not seem to work */

	setenv("TRACE_MSGMAX", "0", 0);
	TRACE_CNTL("mode", 3);
	traceControl_rwp->mode.bits.M= 1;  // NOTE: TRACE_CNTL("modeM",1) hardwired to NOT enable when not mapped!

#define STRT_PRN(fmt2args, a1, a2)     \
	sprintf(buffer, fmt2args, a1, a2); \
	fprintf(stderr, "%-60s", buffer);  \
	fflush(stderr)
	// ELF 6/6/18: GCC v6_3_0 does not like %', removing the '...
#define END_FMT "%10u ns, %9.4f ns/TLOG, %13.3f Mtlogs/s", delta, (double)delta / loops, (double)loops * 1000 / delta
#define CONTINUE                      \
	fprintf(stderr, "Continuing.\n"); \
	continue

	if (args >= 1) tests_mask= (unsigned)strtoul(argv[optind], NULL, 0);
	if (args >= 2) modes_mask= (unsigned)strtoul(argv[optind + 1], NULL, 0);
	struct results_t {
		uint32_t delta;
		unsigned loops;
	};
	unsigned total_tests= count_bits(tests_mask) * count_bits(modes_mask);
	size_t results_bytes= sizeof(results_t) * total_tests;
	results_t *results_a= (results_t *)malloc(results_bytes);
	char prop[]= {'|', '/', '-', '\\'};
	results_t normal= {0, 0};

	unsigned test= 0;
	for (int jj= 0; jj < 4; ++jj) {
		//unsigned tstmod= (1U << jj) & modes_mask;
		switch ((1U << jj) & modes_mask) {
		case 1:
			TRACE_CNTL("lvlclrM", 1LL << TLVL_INFO);  // operate on the TLOG(TLVL_INFO) bit for M
			TRACE_CNTL("lvlclrS", 1LL << TLVL_INFO);  // operate on the TLOG(TLVL_INFO) bit for S
			loops= (unsigned)opt_loops * 5;
			break;
		case 2:
			TRACE_CNTL("lvlsetM", 1LL << TLVL_INFO);
			TRACE_CNTL("lvlclrS", 1LL << TLVL_INFO);
			loops= (unsigned)opt_loops * 2;
			break;
		case 4:
			TRACE_CNTL("lvlsetM", 1LL << TLVL_INFO);
			TRACE_CNTL("lvlsetS", 1LL << TLVL_INFO);
			loops= (unsigned)opt_loops;
			break;
		case 8:
			TRACE_CNTL("lvlclrM", 1LL << TLVL_INFO);
			TRACE_CNTL("lvlsetS", 1LL << TLVL_INFO);
			loops= (unsigned)opt_loops;
			break;
		case 0: continue;  // Should give/have "invalid modes spec" message
		}

		if (1 & tests_mask) {
			//STRT_PRN(" 0x001 -%s const short msg %s", "", (tstmod & 0xc) ? "(NO snprintf)" : "");
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) TLOG(TLVL_INFO) << "any msg";
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (2 & tests_mask) {
			//STRT_PRN(" 0x002 - 1 arg%s%s", "", "");
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) TLOG(TLVL_INFO) << "this is one small param: " << 12345678;
			//TLOG(TLVL_INFO) << "this is one long long long long long long long long long longlong long long long long long long long long long message.";
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (4 & tests_mask) {
			//STRT_PRN(" 0x004 - 2 args%s%s", "", "");
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) TLOG(TLVL_INFO) << "this is 2 params: " << 12345678 << " " << uu;
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (8 & tests_mask) {
			//STRT_PRN(" 0x008 - 8 args (7 ints, 1 float)%s%s", "", "");
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu)
				TLOG(TLVL_INFO) << "this is 8 params: " << 12345678 << " " << uu << " " << uu * 2 << " " << uu + 6 << " "
								<< 12345679 << " " << uu << " " << uu - 7 << " " << (float)uu * 1.5;
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (0x10 & tests_mask) {
			//STRT_PRN(" 0x010 - 8 args (1 ints, 7 float)%s%s", "", "");
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu)
				TLOG(TLVL_INFO) << "this is 8 params: " << 12345678 << " " << (float)uu << " " << (float)uu * 2.5 << " "
								<< (float)uu + 3.14 << " " << (float)12345679 << " " << (float)uu / .25 << " "
								<< (float)uu - 2 * 3.14 << " " << (float)uu * 1.5;
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (0x20 & tests_mask) {
			//STRT_PRN(" 0x020 - snprintf of same 8 args%s%s", "", "");
			TRACE_CNTL("reset");
			unsigned loops_sav= loops;
			loops/= 4;
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) {
				snprintf(buffer, sizeof(buffer), "this is 8 params: %u %g %g %g %g %g %g %g", 12345678, (float)uu, (float)uu * 2.5,
						 (float)uu + 3.14, (float)12345679, (float)uu / .25, (float)uu - 2 * 3.14, (float)uu * 1.5);
				TLOG(TLVL_INFO) << buffer;
			}
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			// if (tstmod&0xc && normal.delta==0) {
			// 	normal = { delta, loops }; // normalize against slowest
			// 	fprintf(stderr, "   tstmod=%u delta=%u loops=%u\n", tstmod, delta, loops );
			// }
			results_a[test++]= {delta, loops};
			loops= loops_sav;
		}

		if (0x40 & tests_mask) {
			//STRT_PRN(" 0x040 -%s const short msg %s", (1 & tests_mask) ? " (repeat)" : "", (tstmod & 0xc) ? "(NO snprintf)" : "");
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) TLOG(TLVL_INFO) << "any msg";
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (0x80 & tests_mask) {
			//STRT_PRN(" 0x080 - 2 args%s%s traceTID=-1 - first TLOG", "", ""); //first TLOG (\"initialization\")
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) {
				TLOG(TLVL_INFO) << "this is 2 params: " << 12345678 << " " << uu;
				traceTID= -1;
			}
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (0x100 & tests_mask) {
			//STRT_PRN(" 0x100 - 2 args%s%s TRACE macro", "", "");
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) { TRACE(TLVL_INFO, "this is 2 params: %d %u", 12345678, uu); }
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (0x200 & tests_mask) {
			//STRT_PRN(" 0x200 - %s%s8 args (7 ints, 1 float) - TLOG_SCOPED() TLOG_ADD", "", "");
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) {
				TLOG_SCOPED(TLVL_INFO)
				TLOG_ADD << "this is 8 params: " << 12345678 << " " << uu << " " << uu * 2 << " " << uu + 6 << " " << 12345679 << " "
					  << uu << " " << uu - 7 << " " << (float)uu * 1.5;
			}
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (0x400 & tests_mask) {
			//STRT_PRN(" 0x400 - %s%s8 args (7 ints, 1 float) - TLOG_SCOPED(){TLOG_ADD}", "", "");
			TRACE_CNTL("reset");
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) {
				TLOG_SCOPED(TLVL_INFO)
				{
					TLOG_ADD << "this is 8 params: " << 12345678;
					TLOG_ADD << " " << uu;
					TLOG_ADD << " " << uu * 2;
					TLOG_ADD << " " << uu + 6;
					TLOG_ADD << " " << 12345679;
					TLOG_ADD << " " << uu;
					TLOG_ADD << " " << uu - 7;
					TLOG_ADD << " " << (float)uu * 1.5;
				}
			}
			delta= (uint32_t)(gettimeofday_ns() - mark);
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}

		if (0x800 & tests_mask) {
#undef TLOG
#undef TLOG_SCOPED_DEBUG
#include <iostream>
#define TLOG(...) \
	if (0) std::cout
#define TLOG_SCOPED_DEBUG(...) \
			for(struct{std::ostream* stmr__;} _trc_={&std::cout}; 0; )
#ifndef __OPTIMIZE__
			//STRT_PRN(" 0x800 - 2 args%s%s ", " - NoTLOG - ", "NOT Optimized.");
#else
			//STRT_PRN(" 0x800 - 2 args%s%s ", " - NoTLOG - ", "OPTIMIZED out.");
#endif  // __OPTIMIZE__
			TRACE_CNTL("reset");
			unsigned total= 0;
			mark= gettimeofday_ns();
			for (unsigned uu= 0; uu < loops; ++uu) {
				TLOG(TLVL_INFO) << "this is 2 params: " << 12345678 << " " << uu;
				TLOG_SCOPED_DEBUG(TLVL_INFO) TLOG_ADD << "this is 2 params: " << 12345678 << " " << uu;
				total+= uu;  // don't want to completely optimize out whole loop
			}
			delta= (uint32_t)(gettimeofday_ns() - mark);
			printf("%u\n", total);  // use the value
			fprintf(stderr, "%c %2u/%u\r", prop[test & 0x3], test + 1, total_tests);
			fflush(stderr);
			results_a[test++]= {delta, loops};
		}
	}  // for (jj<4)

	//===============================================================================
	double highest= 0.0;
	for (unsigned tt= 0; tt < test; ++tt) {
		double this_result= (double)results_a[tt].delta / results_a[tt].loops;
		if (this_result > highest) {
			highest= this_result;
			normal= results_a[tt];
		}
	}
	//fprintf(stderr, "\nnormal.delta=%u normal.loops=%u\n",normal.delta,normal.loops);
	//===============================================================================

	test= 0;
	for (int jj= 0; jj < 4; ++jj) {
		unsigned tstmod= (1U << jj) & modes_mask;
		switch ((1U << jj) & modes_mask) {
		case 1:
			TRACE_CNTL("lvlclrM", 1LL << TLVL_INFO);  // operate on the TLOG(TLVL_INFO) bit for M
			TRACE_CNTL("lvlclrS", 1LL << TLVL_INFO);  // operate on the TLOG(TLVL_INFO) bit for S
			loops= (unsigned)opt_loops * 5;
			fprintf(stderr, "0x1 M0S0 - Testing with M and S lvl disabled. loops=%u\n", loops);
			break;
		case 2:
			TRACE_CNTL("lvlsetM", 1LL << TLVL_INFO);
			TRACE_CNTL("lvlclrS", 1LL << TLVL_INFO);
			loops= (unsigned)opt_loops * 2;
			fprintf(stderr, "0x2 M1S0 - Testing with S lvl disabled (mem only). loops=%u\n", loops);
			break;
		case 4:
			TRACE_CNTL("lvlsetM", 1LL << TLVL_INFO);
			TRACE_CNTL("lvlsetS", 1LL << TLVL_INFO);
			loops= (unsigned)opt_loops;
			fprintf(stderr, "0x4 M1S1 - Testing with M and S lvl enabled (stdout>/dev/null). loops=%u\n", loops);
			break;
		case 8:
			TRACE_CNTL("lvlclrM", 1LL << TLVL_INFO);
			TRACE_CNTL("lvlsetS", 1LL << TLVL_INFO);
			loops= (unsigned)opt_loops;
			fprintf(stderr, "0x8 M0S1 - Testing with just S lvl enabled. Unusual (freeze). loops=%u\n", loops);
			break;
		case 0: continue;  // Should give/have "invalid modes spec" message
		}

		if (1 & tests_mask) {
			STRT_PRN(" 0x001 -%s const short msg %s", "", (tstmod & 0xc) ? "(NO snprintf)" : "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (2 & tests_mask) {
			STRT_PRN(" 0x002 - 1 arg%s%s", "", "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (4 & tests_mask) {
			STRT_PRN(" 0x004 - 2 args%s%s", "", "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (8 & tests_mask) {
			STRT_PRN(" 0x008 - 8 args (7 ints, 1 float)%s%s", "", "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (0x10 & tests_mask) {
			STRT_PRN(" 0x010 - 8 args (1 ints, 7 float)%s%s", "", "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (0x20 & tests_mask) {
			STRT_PRN(" 0x020 - snprintf of same 8 args%s%s", "", "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (0x40 & tests_mask) {
			STRT_PRN(" 0x040 -%s const short msg %s", (1 & tests_mask) ? " (repeat)" : "", (tstmod & 0xc) ? "(NO snprintf)" : "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (0x80 & tests_mask) {
			STRT_PRN(" 0x080 - 2 args%s%s traceTID=-1 - first TLOG", "", "");  //first TLOG (\"initialization\")
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (0x100 & tests_mask) {
			STRT_PRN(" 0x100 - 2 args%s%s TRACE macro", "", "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (0x200 & tests_mask) {
			STRT_PRN(" 0x200 - %s%s8 args (7 ints, 1 float) - TLOG_SCOPED() TLOG_ADD", "", "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (0x400 & tests_mask) {
			STRT_PRN(" 0x400 - %s%s8 args (7 ints, 1 float) - TLOG_SCOPED(){TLOG_ADD}", "", "");
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}

		if (0x800 & tests_mask) {
#ifndef __OPTIMIZE__
			STRT_PRN(" 0x800 - 2 args%s%s ", " - NoTLOG - ", "NOT Optimized.");
#else
			STRT_PRN(" 0x800 - 2 args%s%s ", " - NoTLOG - ", "OPTIMIZED out.");
#endif  // __OPTIMIZE__
			delta= results_a[test++].delta;
			fprintf(stderr, END_FMT);
			if (opt_normalize)
				fprintf(stderr, " %13.3f\n", (double)loops * 1000 / delta / ((double)normal.loops * 1000 / normal.delta));
			else
				fprintf(stderr, "\n");
		}
	}  // for (jj<4)

	return (0);
}  // main
