/*  This file (Trace_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_cntl.c,v $
    rev="$Revision: 1.1 $$Date: 2014/01/15 15:25:03 $";
    */
/*
gxx_standards.sh Trace_test.c
for std in $standards;do\
  for ll in 0 1 2 3;do echo $std $ll:;TRACE_LVL=$ll ./Trace_test-$std;done;\
done
   
*/
#include <stdio.h>		/* printf */
#include "Trace.h"

int
main(  int	argc
     , char	*argv[] )
{
    TRACE( 0, "hello" );
    TRACE( 1, "hello %d", 1 );
    TRACE( 2, "hello %d %d", 1, 2 );
    TRACE( 3, "hello %d %d %d", 1,2,3 );
    return (0);
}   /* main */
