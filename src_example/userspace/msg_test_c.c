/*  This file (msg_test_c.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Aug  8, 2019. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: .emacs.gnu,v $
	rev="$Revision: 1469 $$Date: 2021-01-19 16:44:19 -0600 (Tue, 19 Jan 2021) $";
	*/
/*
    This tests the maximum slow path message buffer. It uses the veriable
    array length feature of C. The default buffer size is small and used to
    test all of the TRACE_PRINT specs. See the USAGE below.
 */
#define USAGE  "\
   usage: %s [msg]\n\
examples:\n\
export TRACE_MSG_MAX=0; treset; TRACE_PRINT='1%%|%%M' %s \"hello${cr}\" -l3 -s1; echo '\\t\\tx'\n\
TRACE_PRINT='123%%M' %s \"hello${cr}\" -m10; echo '\\t\\tx'\n\
TRACE_PRINT='12' %s \"hello${cr}\" -m10; echo '\\t\\tx'\n\
TRACE_PRINT='12' %s \"hellox\" -m10; echo '\\t\\tx'\n\
for cc in S '#S' a C e F f I i L *L l M m N n O o P T t u x;do\n\
  TRACE_PRINT=\"%%$cc\" %s \"hellox\" -m10\n\
done\n\
options:\n\
-l<loops>\n\
-m<output_buffer_size>\n\
-s<sleep_s>\n", basename(argv[0]), basename(argv[0]), basename(argv[0]), basename(argv[0]) \
		,basename(argv[0]), basename(argv[0])

#include <libgen.h>				/* basename */
#include <stdio.h>				/* printf */
#include <stdlib.h>				/* setenv */
#include <unistd.h>				/* getopt */

unsigned msgmax=13;				/* the min length is the strlen of TRACE_PRINT "%T %n %L %M" ==> 11, THEN PLUS ROOM FOR \n\0 */
#define TRACE_USER_MSGMAX msgmax
#include <TRACE/trace.h>

#define ONES(xx) ((xx) - ((xx)/10*10))
// act like snprintf -- if bufsiz is 0: no/zero characters are written
//                      if bufsiz is 1: a null char is written
char *create_ascii_msg( char *buf, size_t bufsiz )
{
	unsigned ii;
	if (bufsiz == 0)
		return (buf);
	// so now bufsiz must be greater than 0 (it's unsigned and can't be negative
	for (ii=0; ii<(bufsiz-1); ++ii) {
		if ((ii+1)%10 == 0)
			buf[ii] = (char)('0' + ONES((ii+1)/10));
		else
			buf[ii] = (char)('0' + ONES((ii+1)));
	}
	buf[ii]='\0';
	return (buf);
}

#pragma GCC diagnostic ignored "-Wformat-security"

int main( int argc, char *argv[] )
{
	unsigned ii, xx;
	char     *no_args_msg;
	char     buf[0x1800];
	int      opt, args;			/* for how I use getopt */
	//unsigned opt_msgmax=0;		/* a zero would be impossible */
	int      opt_loops=1;
	unsigned opt_sleep=0;
extern char       *optarg;        /* for getopt */
extern int        optind;         /* for getopt */
extern int getopt(int, char * const [], const char *);
	
    while ((opt=getopt(argc,argv,"?hl:m:s:")) != -1){
		switch (opt){
		/* '?' is also what you get w/ "invalid option -- -" */
		case '?': case 'h': printf( USAGE ); exit (0); break;
		case 'l': opt_loops = (int)strtoul(optarg, NULL, 0); break;
        case 'm': msgmax = (unsigned)strtoul(optarg, NULL, 0); break;
		case 's': opt_sleep = (unsigned)strtoul(optarg, NULL, 0); break;
		}
	}
	args = argc - optind;
	TRACE_CNTL("init");			/* make sure tracePrint_cntl is set */


	//setenv("TRACE_MSGMAX","0",0);
	printf("w/ %7s=tracePrint_cntl and obuf[%d]: ",tracePrint_cntl,msgmax); fflush(stdout);
	if (args == 1) {
		for (int ii=0; ii<opt_loops; ++ii) {
			TRACE(TLVL_INFO, argv[optind]);
			if (opt_sleep && ii<opt_loops)
				sleep(opt_sleep);
		}
		return (0);
	} else
		TRACE(2, "hello, hi there" );

	for (xx=msgmax+1; xx<18; ++xx) {
		msgmax=xx+56;
		printf("w/obuf[%d]:\n",msgmax);
		TRACE(2,"***>>>---> xx=%u <---<<<***",xx);
		msgmax=xx;
		for (ii=0; ii<2; ++ii) {
			printf("w/obuf[%d]:\n",msgmax);
			TRACE(2,"hello, ii=%u", ii );
		}
	}
	printf("w/obuf[%d]:\n",msgmax);
	TRACE( 2, "hello, in the middle");


	tracePrint_cntl="";  // just message should be printed
	msgmax=0x1800;
	printf("w/obuf[%d]:\n",msgmax);
	no_args_msg="changed tracePrint_cntl to \"\" which gets changed to \"%m\" in vtrace_user";
	TRACE(2, no_args_msg /*no args prints raw msg*/);
	for (xx=3; xx<14; ++xx) {   // 3 would be sizeof("%m") OR strlen("%m")+1
		msgmax=xx+22;
		printf("w/obuf[%d]:\n",msgmax);
		TRACE(2,"***>>>---> xx=%u <---<<<***",xx);
		msgmax=xx;
		for (ii=0; ii<2; ++ii) {
			printf("w/obuf[%d]:\n",msgmax);
			TRACE(2,"hi, ii=%u", ii );
		}
	}

	tracePrint_cntl="%T %n %L %M";
	msgmax=0x1800;
	no_args_msg="changed back to defaults: msgmax=0x1800 tracePrint_cntl=\"%T %n %L %M\"";
	printf("w/obuf[%d]:\n",msgmax);
	TRACE( 2, no_args_msg );
	TRACE( 2, create_ascii_msg(buf,81) );

	TRACE( 2, "the max msg len in the default env depends on the strlen tracePrint_cntl and traceControl_rwp->longest_name=%u"
	      , traceControl_rwp->longest_name );
	for (xx=0x1800-48; xx<0x1800-45; ++xx) {
		TRACE(2,"***>>>---> xx=%u <---<<<***",xx);
		TRACE(2, create_ascii_msg(buf,xx) );
	}

	no_args_msg="one big message with something on the end: msgmax=0x1800 tracePrint_cntl=\"%T %n %L %M\n%%MSG\"";
	TRACE( 2, no_args_msg );
	tracePrint_cntl="%T %n %L %M\n%%MSG";
	TRACE( 2, create_ascii_msg(buf,0x1800-45) );

	tracePrint_cntl="%T %n %L %M";
	TRACE( 2, "messages with \\r" );
	for (ii=5; ii; --ii) {
		TRACE( 2, "ii=%u      \r", ii );
		sleep( 1 );
	}
	printf("w/obuf[%d]:\n",msgmax);
	TRACE( 2, "ii=%u      ", ii );
	return (0);
}   /* main */
