 // This file (no_trace.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Aug 22, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";

// g++ -g -O0 -Wall -std=c++11 -o trace_disabled{,.cc} && ./trace_disabled

# include <unistd.h>			// write
# include <sstream>				// std::ostringstream

#define NO_TRACE
#ifndef NO_TRACE

# include <iostream>			// std::cout
# define TLOG(...)      if(0)std::cout
# define TRACE(...)
# define TRACEN(...)
# define TRACEN_(...)
# define TRACE_CNTL(...) (0L)

#else

# include <unistd.h>			// write
# include <sstream>				// std::ostringstream
# define TRACE_DEBUG_LVL 2
# define TRACE_MSG_MAX   0x200
# define TARG1(a1, ...) a1
# define TLOG(...)      for( std::ostringstream ss; \
	                         TARG1(__VA_ARGS__,need_at_least_one)<=TRACE_DEBUG_LVL && ss.str().size()==0; \
							 ss.str().back()!='\n'&&ss << "\n", write(1,&ss.str()[0],ss.str().size()) )	\
		                    ss
# define TRACE(lvl,...) do if(lvl<=TRACE_DEBUG_LVL){ /*wrap in do...while(0) for certain 'if' syntax */ \
							char obuf[TRACE_MSG_MAX]; \
							int nn=snprintf(obuf,sizeof(obuf),__VA_ARGS__);	\
							if(nn>=(int)sizeof(obuf)){/*truncated(but still terminated)*/ \
								obuf[sizeof(obuf)-2]='\n';				\
								nn=sizeof(obuf)-1;						\
							} else if(obuf[nn-1]!='\n' && nn==(sizeof(obuf)-1)) \
								obuf[nn-1]='\n';						\
							else if(obuf[nn-1]!='\n')					\
								obuf[nn++]='\n';						\
							write(1,obuf,nn);							\
						} while(0)
# define TRACEN( nam,lvl,... ) TRACE(lvl,__VA_ARGS__)
# define TARGS( arg1, ... ) __VA_ARGS__
# define TRACEN_( nam, lvl,... ) do if(lvl<=TRACE_DEBUG_LVL){ \
									std::ostringstream ostr__;			   \
									ostr__ << TARG1(__VA_ARGS__,not_used); \
									TRACE( lvl, &ostr__.str()[0], TARGS(__VA_ARGS__,0/*potential xtra 0 OK*/));	\
								 } while(0)
# define TRACE_CNTL(...) (0L)

#endif


int main( int argc, char *argv[] )
{
	
	for (std::stringstream ss;
		 2<=2 && ss.str().size()==0;
		 ss.str().back()!='\n'&&ss << "\n", write(1,&ss.str()[0],ss.str().size()) )
		ss << "hello from " << argv[0];

	if(2<=2){
		char obuf[20];
		int nn=snprintf(obuf,sizeof(obuf),"hello argc=%d",argc);
		if(nn>=(int)sizeof(obuf)){/*truncated(but still terminated)*/
			obuf[sizeof(obuf)-2]='\n';
			nn=sizeof(obuf)-1;
		} else if(obuf[nn-1]!='\n' && nn==(sizeof(obuf)-1))
			obuf[nn-1]='\n';
		else if(obuf[nn-1]!='\n')
			obuf[nn++]='\n';
		write(1,obuf,nn);
	}

	TLOG(2) << "hi " << 2 << " there\n";
	TRACE( 2, "hi %d there", 3 );
	TRACEN_( "notUsed", 2, "hi %d there " << "joe", 55 ); 
	if (1)
		TRACE( 2, "in if statement" );
	else
		TRACE( 2, "in if else" );
	if (1)
		TRACEN_("notUsed",2,"hi %d there, then", 44 );
	else
		TRACE( 2, "in if else" );
	TLOG(3,"name") << "don't see this";
	return (0);
}   // main
