/*  This file (basic_c.c) was created by Ron Rechenmacher <ron@fnal.gov> on
	Dec 13, 2015. "TERMS AND CONDITIONS" governing this file are in the README
	or COPYING file. If you do not have such a file, one can be obtained by
	contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
	$RCSfile: .emacs.gnu,v $
	rev="$Revision: 1.26 $$Date: 2015/08/02 00:03:01 $";
	*/

#define _GNU_SOURCE
#include <stdio.h>		/* printf */
struct basic_c_s
{ char ver1[0x1000];
  char ver2[0x1000];
};

static struct basic_c_s  before = {{0},{0}};
#include "trace.h"		/* TRACE */
static struct basic_c_s  after = {{0},{0}};

int main( )
{
	unsigned ii;
	unsigned char *byte_ptr;
	printf("printf - main start\n");

	TRACE( 10,"TRACE - 0 args");
	TRACE( 20,"TRACE - 35 args "
		  "%d %d %d %d %d %d %d %d %d %d "
		  "%d %d %d %d %d %d %d %d %d %d "
		  "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		  , 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 );

	/* Currently, TRACE supports 0-35 args after the format, if you accidently put 36 args,
	   the trace macros thinks the number of args is the 36th arg :(
	   This example also shows that if one accidentally puts a lvl > 63, the macro mask the lower bits */
	TRACE( 30+64,"TRACE - 35 args "
		  "%d %d %d %d %d %d %d %d %d %d "
		  "%d %d %d %d %d %d %d %d %d %d "
		  "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		  , 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,6 );

	printf("\
      &before[0]=%p\n\
&traceControl[0]=%p\n\
       &after[0]=%p\n\
",(void*)&before,(void*)&traceControl[0],(void*)&after );

    /* The following is meant to test the trace "disabled" case --
	   to make sure that no writing beyond the trace variable storage occurs.
	   With the 2016.01.21 (revision 486) of the trace.h file, changing the
       size of the disabled traceControl variable from 2 to 1 will cause
	   overwriting of the "after" variable.
	 */
	byte_ptr = (unsigned char *)&before;
	for (ii=0; ii<sizeof(before); ++ii)
		if (byte_ptr[ii] != 0) { printf("problem with before\n"); break; }
	if(ii == sizeof(before)) printf("before OK\n");
	byte_ptr = (unsigned char *)&after;
	for (ii=0; ii<sizeof(after); ++ii)
		if (byte_ptr[ii] != 0) { printf("problem with after\n"); break; }
	if(ii == sizeof(after)) printf("after OK\n");

	printf("printf - main done\n");
	return (0);
}   /* main */
