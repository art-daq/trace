/*  This file (basic_c.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Dec 13, 2015. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: .emacs.gnu,v $
	rev="$Revision: 1.26 $$Date: 2015/08/02 00:03:01 $";
	*/

#define _GNU_SOURCE
#include <stdio.h>		/* printf */
#include "trace.h"		/* TRACE */

int main( )
{
	printf("printf - main start\n");
	TRACE( 0,"TRACE - 0 args");
	TRACE( 0,"TRACE - 35 args "
		  "%d %d %d %d %d %d %d %d %d %d "
		  "%d %d %d %d %d %d %d %d %d %d "
		  "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		  , 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 );

	/* Currently, TRACE supports 0-35 args after the format, if you accidently put 36 args,
	   the trace macros thing the number of arg is the 36th arg :( */
	TRACE( 0,"TRACE - 35 args "
		  "%d %d %d %d %d %d %d %d %d %d "
		  "%d %d %d %d %d %d %d %d %d %d "
		  "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		  , 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,6 );
	printf("printf - main done\n");
	return (0);
}   /* main */
