 // This file (TLOG_types.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // Mar 24, 2020. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1304 $$Date: 2020-04-13 01:26:17 -0500 (Mon, 13 Apr 2020) $";

// demonstrate the 21 delayable types plus 1 custome delayable 
#include "TRACE/trace.h"		// TLOG

struct myObj {
	int x;
    int y;
};

inline TraceStreamer & operator<<(TraceStreamer& x, myObj &r)
{
    x.msg_append("x: ");
	x.delay_format(r.x);
	x << " y: ";				// pretty much the same as msg_append
	x.delay_format(r.y);
	return x;
}

struct myObj2 {
	int x;
	int y;
	friend std::ostream& operator<<(std::ostream& os, myObj2 const& r) {
		os << "X: " << r.x << " y: " << r.y;
		return os;
	}
	operator std::string() {
		return std::string("X: ")+std::to_string(x)+ " Y: "+std::to_string(y);
	}
};


int main()
{
	const int *xx1 = (int*)0x12345678; 	TLOG(TLVL_DEBUG) << "1  const T *const &r " << xx1;
	int *const xx2 = (int*)0x12345678;  TLOG(TLVL_DEBUG) << "2  T *const &r " << xx2;
	char xx3=' ';                       TLOG(TLVL_DEBUG) << "3  const char &r " << xx3;
	unsigned char xx4=44;               TLOG(TLVL_DEBUG) << "4  const unsigned char &r " << xx4;
	int xx5=5;                          TLOG(TLVL_DEBUG) << "5  const int &r " << xx5;
	short int xx6=6;                    TLOG(TLVL_DEBUG) << "6  const short int &r " << xx6;
	long int  xx7=7777;                 TLOG(TLVL_DEBUG) << "7  const long int &r " << xx7;
	short unsigned int xx8=0xffff;      TLOG(TLVL_DEBUG) << "8  const short unsigned int &r " << std::hex << xx8;
	unsigned int xx9=9999;              TLOG(TLVL_DEBUG) << "9  const unsigned int &r " << xx9;
	long unsigned int xx10=10;          TLOG(TLVL_DEBUG) << "10 const long unsigned int &r " << xx10;
	long long unsigned int xx11=11;     TLOG(TLVL_DEBUG) << "11 const long long unsigned &r " << xx11;
	double xx12=12.0;                   TLOG(TLVL_DEBUG) << "12 const double &r " << xx12;
	long double xx13=13.0;              TLOG(TLVL_DEBUG) << "13 const long double &r " << xx13;
	float xx14=14.0;                    TLOG(TLVL_DEBUG) << "14 const float &r " << xx14;
	bool xx15=true;                     TLOG(TLVL_DEBUG) << "15 const bool &r " << xx15;
#	if (__cplusplus >= 201103L)
	std::atomic<int> xx16(16);          TLOG(TLVL_DEBUG) << "16 const std::atomic<int> &r " << xx16;
	std::atomic<unsigned long> xx17(17);TLOG(TLVL_DEBUG) << "17 std::atomic<unsigned long> const &r " << xx17;
	std::atomic<short int> xx18(18);    TLOG(TLVL_DEBUG) << "18 std::atomic<short int> const &r " << xx18;
	std::atomic<bool> xx19(true);       TLOG(TLVL_DEBUG) << "19 std::atomic<bool> const &r " << xx19;
#   endif
	std::unique_ptr<std::string> xx20(new std::string("hi"));
	                                    TLOG(TLVL_DEBUG) << "20 std::unique_ptr<T> const &r " << xx20;
	void *xx21=(void*)0xffff1234;       TLOG(TLVL_DEBUG) << "21 void *const  &r " << xx21;

	myObj xx22={1,2};                   TLOG(TLVL_DEBUG) << "22 myObj " << xx22;
	myObj2 xx23={3,4};                  TLOG(TLVL_DEBUG) << "23 myObj2 " << xx23 << " again: " << std::string(xx23);

	return (0);
}   // main
