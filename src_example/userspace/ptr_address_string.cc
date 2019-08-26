 // This file (ptr_address.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Feb 28, 2019. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.30 $$Date: 2016/03/01 14:27:27 $";
#if 0
rm -f /tmp/trace_buffer_`whoami`

/home/ron/work/tracePrj/trace
ron@mu2edaq01 :^) treset; ptr_address_string 
02-28 16:42:54.035092 ptr_address_string nfo TRACE_NAME is ptr_address_string
02-28 16:42:54.035184 ptr_address_string nfo the real message is hello
02-28 16:42:54.035190 ptr_address_string nfo the copy message is hello
02-28 16:42:54.035197 ptr_address_string nfo the address of message string - (void*)real_msg=0x40872e
02-28 16:42:54.035202 ptr_address_string nfo ip=0xc4ac20
02-28 16:42:54.035207 ptr_address_string nfo ullp=0xc4acb0 *ullp=555
02-28 16:42:54.035211 ptr_address_string nfo std::string("hello")=hello
02-28 16:42:54.035215 ptr_address_string nfo unique_ptr up_string=0xc4acd0 which is ptr to std::string (not the actual string within)
02-28 16:42:54.035219 ptr_address_string nfo unique_ptr *up_string=new string
02-28 16:42:54.035223 ptr_address_string nfo unique_ptr (*up_string).c_str()=new string
02-28 16:42:54.035227 ptr_address_string nfo unique_ptr (void*)((*up_string).c_str())=0xc4ace0
02-28 16:42:54.035230 ptr_address_string nfo unique_ptr &(*up_string)[0]=new string
02-28 16:42:54.035234 ptr_address_string nfo unique_ptr (void*)&(*up_string)[0]=0xc4ace0
02-28 16:42:54.035244 ptr_address_string nfo unique_ptr &(*up_vector_int)[0]=0xc4ad20
02-28 16:42:54.035249 ptr_address_string nfo unique_ptr (*up_vector_int)[0]=1
--2019-02-28_16:42:54--
#endif

#include <stdio.h>				// printf
#include <stdlib.h>				// malloc
#include <string.h>				// rindex
#include "TRACE/trace.h"		// TRACE
#include <memory>				// std::unique_ptr
#include <vector>				// std::vector
#if __cplusplus >= 201103L
# define TRACE_NAME [](const char *path){const char *bp=rindex(path,'/');	\
										std::string ss((bp!=NULL)?bp+1:path); \
										size_t lastindex = ss.find_last_of("."); \
										return ss.substr(0,lastindex);}(__BASE_FILE__).c_str()
#else
# define TRACE_NAME "ptr_address_string"
#endif

int main()
{
	const char *real_msg="hello";
	int *ip=(int*)malloc(sizeof(int));
	char *cp=(char*)malloc(100);
	strncpy(cp,real_msg,99);cp[99]='\0';
	unsigned long long *ullp=(unsigned long long *)malloc(sizeof(unsigned long long));
	*ullp=555;

	TLOG(TLVL_INFO) << "TRACE_NAME is " << TRACE_NAME;
	TLOG(TLVL_INFO) << "the real message is " << real_msg;
	TLOG(TLVL_INFO) << "the copy message is " << cp;
	TLOG(TLVL_INFO) << "the address of message string - (void*)real_msg=" << (void*)real_msg;
	TLOG(TLVL_INFO) << "ip=" << ip;
	TLOG(TLVL_INFO) << "ullp=" << ullp << " *ullp="<<*ullp;
	TLOG(TLVL_INFO) << "std::string(\"hello\")="<<std::string("hello");
#	if __cplusplus >= 201103L
	std::unique_ptr<std::string> up_string{new std::string("new string")};
	std::unique_ptr<std::vector<int>> up_vector_int{new std::vector<int>{1,2,3}};
	TLOG(TLVL_INFO) << "unique_ptr up_string="<< up_string << " which is ptr to std::string (not the actual string within)";
	TLOG(TLVL_INFO) << "unique_ptr *up_string="<< *up_string;
	TLOG(TLVL_INFO) << "unique_ptr (*up_string).c_str()="<< (*up_string).c_str();
	TLOG(TLVL_INFO) << "unique_ptr (void*)((*up_string).c_str())="<< (void*)((*up_string).c_str());
	TLOG(TLVL_INFO) << "unique_ptr &(*up_string)[0]="<<&(*up_string)[0];
	TLOG(TLVL_INFO) << "unique_ptr (void*)&(*up_string)[0]="<<(void*)&(*up_string)[0];
	TLOG(TLVL_INFO) << "unique_ptr &(*up_vector_int)[0]="<<&(*up_vector_int)[0];
	TLOG(TLVL_INFO) << "unique_ptr (*up_vector_int)[0]="<<(*up_vector_int)[0];
#	endif
	return (0);
}   // main
