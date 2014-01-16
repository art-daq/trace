/*  This file (Trace_test.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 15, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_cntl.c,v $
    rev="$Revision: 1.4 $$Date: 2014/01/16 06:36:08 $";
    */
/*
gxx_standards.sh Trace_test.c
for std in $standards;do\
  for ll in 0 1 2 3;do echo $std $ll:;TRACE_LVL=$ll ./Trace_test-$std;done;\
done
   
*/
#include <stdio.h>		/* printf */
#include <libgen.h>		/* basename */
#include "Trace_mmap.h"

#define USAGE "\
%s <cmd> [command opt/args]\n\
", basename(argv[0])

int
main(  int	argc
     , char	*argv[] )
{
    unsigned ii;

#  if 0
    int         opt;
    while (1)
    {   int option_index = 0;
        static struct option long_options[] =
        {   /* name,   has_arg, int* flag, val_for_flag */
            {0,            0,      0,          0}
        };
        opt = getopt_long (argc, argv, "?hn:s:p",
                        long_options, &option_index);
        if (opt == -1) break;
        switch (opt)
        {
        case '?': case 'h':  printf( USAGE );  exit( 0 ); break;
        default:  printf ("?? getopt returned character code 0%o ??\n", opt);
        }
    }
    if (argc - optind != 1)
    {   printf( "Need cmd\n" );
        printf( USAGE ); exit( 0 );
    }
#  endif

    printf( "(unsigned long)-1=0x%lx\n", (unsigned long)-1 );

    if (argc <= 1) { printf(USAGE); exit(0); }

    if      (strncmp(argv[1],"test",4) == 0)
    {
	TRACE( 0, "hello" );
	TRACE( 1, "hello %d", 1 );
	TRACE( 2, "hello %d %d", 1, 2 );
	TRACE( 3, "hello %d %d %d", 1,2,3 );

#      ifndef TEST_UNUSED_FUNCTION
	TRACE_CNTL( "trig", -1, 5 );
#      endif
	for (ii=0; ii<20; ++ii)
	    TRACE( 0, "ii=%u", ii );
    }
    else if (strncmp(argv[1],"info",4) == 0) 
    {
	TRACE_CNTL( argv[1] );
    }
    else if (strcmp(argv[1],"reset") == 0) 
    {
	TRACE_CNTL( argv[1] );
    }
    else
    {   printf("invalid command\n" USAGE );
    }
    return (0);
}   /* main */
