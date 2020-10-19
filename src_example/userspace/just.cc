/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just.cc,v $
// rev="$Revision: 1385 $$Date: 2020-09-25 13:58:09 -0500 (Fri, 25 Sep 2020) $";
*/

#include <string>
#include "TRACE/trace.h"		/* TRACE */

void example_sub_( void );

static void sub0()
{
    TRACE( TLVL_LOG, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
}

static void sub1()
{
    //TRACE( TLVL_INFO, std::string("hi %d")+" there", 2 );
    TRACE( TLVL_LOG, "hi %d - string param: "+std::string("there"), 2 );
}

int main(/*int argc, char *argv[]*/)
{

	sub0();
	sub1();
	TLOG(TLVL_INFO) << "hi from TLOG";
    example_sub_();
    return (0);
}   /* main */
