/* This file (just.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// Feb 19, 2014. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: just_user.cc,v $
*/
char const *rev="$Revision: 1208 $$Date: 2019-10-02 15:07:23 -0500 (Wed, 02 Oct 2019) $";


#include <stdarg.h>		/* va_list */
#include <string>
#include <getopt.h>             // getopt_long, {no,required,optional}_argument, extern char *optarg; extern int opt{ind,err,opt}
#include <sstream>
#include <ios>
#include <iostream>				// std::cout
#include <typeinfo>				// typeid().name()
#include <libgen.h>				// basename
#include <stdlib.h>
#include <stdint.h>				// uint16_t

#if 1   /* set to 0 to test trace.h TRACE_LOG_FUNCTION */
void my_log(uint16_t nargs, std::string  msg,...);
void my_log2(timeval *, int, short unsigned int, const char *, const char *, int, short unsigned int nargs, const char * msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	my_log(nargs, msg, ap);
	va_end(ap);
}
void my_log2(timeval *, int, short unsigned int, const char *, const char *, int, short unsigned int nargs, std::string msg, ...)
{
	va_list ap;
	va_start(ap, msg);
	my_log(nargs, msg, ap);
	va_end(ap);
}
# define TRACE_LOG_FUNCTION  my_log2
#endif

#include "TRACE/trace.h"		/* TRACE */

#define USAGE "\
  usage: %s [-t <test>]\n\
\n\
example: %s -t_\n\
         %s\n\
\n\
options:\n\
 --help\n\
	-t<test>\n\
", basename(argv[0]), basename(argv[0]), basename(argv[0])
#define VUSAGE "\n"

/* GLOBALS */
static int           opt_v=0;
static char const   *opt_test="";
static int			 opt_loops=10000;

// Note: "static" is good practice as compiler will warn when not used.
// "_" at need to distinguish my functions from other library functions.
static void parse_args_( int argc, char	*argv[] )
{
	// parse opt, optargs, and args
	while (1) {
		int opt;
		static struct option long_options[] = {
			// name     has_arg          *flag  val
			{ "help",   no_argument,      0,    'h' },
			{ "verbose",optional_argument,0,    'v' }, // --verbose can take optional arg (not so for just -v, so -vvv is allowed)
			{     0,    0,                0,    0 }
		};
		opt = getopt_long( argc, argv, "?hvVl:t:", // optional args use "::".  NOTE: -v has no arg but --verbose can have optional
						  long_options, NULL );
		if (opt == -1) break;
		switch (opt) {
		case '?': case 'h': printf(USAGE);if(opt_v)printf(VUSAGE);exit(0);break;
		case 'V': printf("%s\n",rev);       exit(0);                     break;
		case 'v': if(optarg)opt_v=strtoul(optarg,0,0); else ++opt_v;    break; // optional is for when coming from long option
		case 'l': opt_loops=strtoul(optarg,NULL,0);                     break;
		case 't': opt_test=optarg;                                      break;
		default:
			printf( "?? getopt returned character code 0%o ??\n", opt );
			exit( 1 );
		}
	}
}



SUPPRESS_NOT_USED_WARN
void my_log(uint16_t nargs, const char   *msg,...)
{ va_list ap;
  va_start(ap,msg);
  if(nargs)vprintf(msg,ap);
  else      printf("%s",msg);
  printf("\n");
  va_end(ap);
}
SUPPRESS_NOT_USED_WARN
void my_log(uint16_t nargs, std::string  msg,...)
{ va_list ap;
  va_start(ap,msg);
  if(nargs)vprintf(msg.c_str(),ap);
  else     printf("%s",msg.c_str());
  printf("\n");
  va_end(ap);
}


void test( std::ostream &arg )
{   std::cout << "test start\n";
    std::ostringstream& os = dynamic_cast<std::ostringstream&>(arg);
	std::cout << os.str() << "\n";
	std::cout << "test done\n";
}

// this doesn't work with test( ostr << '\n' )
void test( std::ostringstream &arg )
{   std::cout << "test start\n";
	std::cout << arg.str() << "\n";
	std::cout << "test done\n";
}


int main( int argc, char *argv[] )
{
	parse_args_( argc, argv );
	std::string stdstr("hello my name is");
	std::ostringstream ostr("inital characters123");
	char const *filename="some/path";
	std::string fname("some/path2");

	if (strcmp(opt_test,"") == 0) {
		std::cout << "hello\n";
		//TRACE(1,std::cout << "hello"); // THIS prints hello to screen and an address -- not useful

		TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,2 );
		TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.5,3 );

		TLOG_DBG( 1,"TRACE") << stdstr << " ron";
		TRACE( 1, "ron is my name." );

		// now with args
		//TRACE_( 1, stdstr << " ron %d", 1 );
		TRACE( 1, "ron is my name." );
		TLOG_DBG( 1,"TRACE")<< "hello - nice for strings: file="<<filename<<" "<<1<<" "<<1.6<<" "<<2;
		TRACE( 1, "hello - nice for strings: file="+fname+" %d %.1f %d",1,1.6,2 );
		TLOG_DBG( 1,"TRACE")<< "hello - hopefully no compile warnings "<<1<<" "<<1.6<<" "<<std::hex<<15;

		std::cout<<"\n";

		//FILE *fp(popen("head -107 $HOME/.bashrc", "r")); // 107 lines is 4025 chars - works
		//FILE *fp(popen("head -108 $HOME/.bashrc", "r")); // 108 lines is 4192 chars - works
		//FILE *fp(popen("head -115 $HOME/.bashrc", "r")); // 115 lines is 4754 chars - works
		FILE *fp(popen("head -525 $HOME/.bashrc", "r")); // 525 lines is 22843 chars - crash
		//FILE *fp(popen("head -170 $HOME/.bashrc", "r")); // 170 lines is 8258 chars - works
		//FILE *fp(popen("head -300 $HOME/.bashrc", "r")); // works
		//FILE *fp(popen("head -400 $HOME/.bashrc", "r")); // crash
		//FILE *fp(popen("head -350 $HOME/.bashrc", "r")); // works
		//FILE *fp(popen("head -375 $HOME/.bashrc", "r")); // works
		//FILE *fp(popen("head -388 $HOME/.bashrc", "r")); // crash
		//FILE *fp(popen("head -382 $HOME/.bashrc", "r")); // crash
		//FILE *fp(popen("head -378 $HOME/.bashrc", "r")); // works  378 lines is 16664
		//FILE *fp(popen("head -380 $HOME/.bashrc", "r")); // crash
		//FILE *fp(popen("head -379 $HOME/.bashrc", "r")); // crash  379 lines in 16742 -- line 379 has HISTTIMEFORMAT="%a %m/%d %H:%M:%S  "
		//FILE *fp(popen("cat $HOME/t.t", "r")); // 63125 KB of lines of 100 chars 123456... - works
		std::string result;
		if (fp) {
			char buffer[128];
			while (!feof(fp)) {
				if (fgets(buffer, 128, fp) != NULL)
					result += buffer;
			}
			TLOG_DBG( 1,"TRACE")<< "bashrc: " << result;
			pclose(fp);
		}

		ostr << "<< stdstr (just to confuse) %d" << stdstr;
		std::cout << ostr.str() << "\n";
		test( ostr );
		test( ostr << "test function passed ostr<<\"string\"\n" );

# if defined(__cplusplus)&&(__cplusplus>201103L)
#  if 0
		test( (std::ostringstream)"cast (std::ostringstream)\"str\"<<\"str\"" << "jkl;\n" ); // gives address
		test( std::ostringstream("asdf") << "std::ostringstream(\"asdf\")<<\"string\"\n" ); // also gives address
#  endif
# endif

		//std::cout << typeid( std::ostringstream("asdf") << "xyz\n" ).name() << '\n';
		test( std::ostringstream("asdf").flush() << "xyz\n" ); // gives desired output

		std::cout<<"\nXXXX\n";

		//TRACE_( 1, ostr, 1 ); // an address  (also the 1 param is extra)
		TRACE( 1, ostr.str(), 1 );


		std::cout<<"\n";

		//TRACE( 1, ostr <<"I would like to see what is it, not it's address", 1 ); // THIS prints to the screen and prints an address -- not useful
		//TRACE_( 1, (ostr<<"xx").str(), 2 ); these shenanigans don't compile

		TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.7,2 );
		TRACE( 0, "hello - hopefully no compile warnings %d %.1f %d",1,1.7,3 );

		TRACE( 1,
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		   , 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 );
		TRACE( 1,
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d "
		   "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
		   , 41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75 );
		TLOG(1)<< 0<<" "<< 1<<" "<< 2<<" "<< 3<<" "<< 4<<" "<< 5<<" "<< 6<<" "<< 7<<" "<< 8<<" "<< 9<<" "
			   <<10<<" "<<11<<" "<<12<<" "<<13<<" "<<14<<" "<<15<<" "<<16<<" "<<17<<" "<<18<<" "<<19<<" "
			   <<20<<" "<<21<<" "<<22<<" "<<23<<" "<<24<<" "<<25<<" "<<26<<" "<<27<<" "<<28<<" "<<29<<" "
			   <<30<<" "<<31<<" "<<32<<" "<<33<<" "<<34;
	}
	else if (strcmp(opt_test,"<")==0) {
		while(opt_loops--) {
			TLOG_DBG(0,"TRACE")<<"this is an int: " << 55;
		}
	}
	else if (strcmp(opt_test,"C")==0) {
		while(opt_loops--) {
			TRACE(0,"this is an int: %d",55 );
		}
	}
# if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
	else if (strcmp(opt_test,"S")==0) {
		while(opt_loops--) {
			TRACE(0,"this is an int: "+std::to_string(55LL) );// this does same/similar thing as "<<" into stream (but without the stream constructor overhead
		}
	}
# endif
    else {
		printf("invalid -t option %s (could be because of compiler version used)\n", opt_test );
	}

	return (0);
}   /* main */
