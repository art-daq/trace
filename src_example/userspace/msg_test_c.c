/*  This file (msg_test_c.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Aug  8, 2019. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: .emacs.gnu,v $
	rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";
	*/

#include <stdio.h>		/* printf */

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
			buf[ii] = '0' + ONES((ii+1)/10);
		else
			buf[ii] = '0' + ONES((ii+1));
	}
	buf[ii]='\0';
	return (buf);
}

int
main(  int	argc
     , char	*argv[] )
{
	unsigned ii, xx;
	char *no_args_msg;
	char buf[0x1800];

	TRACE(2, "hello, hi there" );
	for (xx=msgmax+1; xx<18; ++xx) {
		msgmax=xx+56;
		TRACE(2,"***>>>---> xx=%u <---<<<***",xx);
		msgmax=xx;
		for (ii=0; ii<2; ++ii)
			TRACE(2,"hello, ii=%u", ii );
	}
	TRACE( 2, "hello, in the middle");


	tracePrint_cntl="";  // just message should be printed
	msgmax=0x1800;
	TRACE( 2, "changed tracePrint_cntl to \"\" which gets change to \"%m\" in vtrace_user");
	for (xx=3; xx<14; ++xx) {   // 3 would be sizeof("%m") OR strlen("%m")+1
		msgmax=xx+22;
		TRACE(2,"***>>>---> xx=%u <---<<<***",xx);
		msgmax=xx;
		for (ii=0; ii<2; ++ii)
			TRACE(2,"hi, ii=%u", ii );
	}

	tracePrint_cntl="%T %n %L %M";
	msgmax=0x1800;
	no_args_msg="changed back to defaults: msgmax=0x1800 tracePrint_cntl=\"%T %n %L %M\"";
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
	TRACE( 2, "ii=%u      ", ii );
	return (0);
}   /* main */
