

/*  gcc -g -Wall -I$TRACE_INC -o libtracelib.so -shared -fPIC tracelib.c
 OR gcc -std=c11 -Wall -I$TRACE_INC -Wextra -pedantic -c -fPIC -o tracelib.{o,c}
    gcc -shared tracelib.o -o libtracelib.so
 */
#define TRACE_IMPL
#include "TRACE/trace.h"

void ctrace0( char *namp, uint8_t lvl, int32_t line, const char *function, const char *msg)
{
	trace_tv_t tv = {0,0};
	unsigned long long no_args[1];
	va_list ap= TRACE_VA_LIST_INIT((void *)no_args);
	if TRACE_INIT_CHECK(TRACE_NAME) {
		int trcId = (int)trace_name2TID(namp);
		vtrace(&tv, trcId, lvl, line, function, 0, msg, ap);
	}
}


/* similar to TRACE macro, except this compilation unit is for all python
   traces. tid_ cannot be static :(
 */
#undef TRACE
void TRACE( char *namp, uint8_t lvl, int32_t line, const char *function, const char *msg)
{
	struct { char tn[TRACE_TN_BUFSZ];	} _trc_;
	if TRACE_INIT_CHECK(trace_name(TRACE_NAME,__TRACE_FILE__,_trc_.tn,sizeof(_trc_.tn))) {
		/*static TRACE_THREAD_LOCAL*/ int tid_ = -1;
		trace_tv_t lclTime;
		uint8_t lvl_ = (uint8_t)(lvl);
		TRACE_SBUFDECL;
		if (tid_ == -1) tid_ = (int)trace_name2TID(&(namp)[0]);
		lclTime.tv_sec = 0;
		if (traceControl_rwp->mode.bits.M && (traceLvls_p[tid_].M & TLVLMSK(lvl_))) {
			/* Note: CANNOT add to "...NARGS..." (i.e. for long doubles issue) b/c nargs==0 in mem entry is signficant */
			trace(&lclTime, tid_, lvl_, line, function, 0 TRACE_XTRA_PASSED, msg);
		}
		if (traceControl_rwp->mode.bits.S && (traceLvls_p[tid_].S & TLVLMSK(lvl_))) {
			TRACE_LIMIT_SLOW(lvl_, _insert, &lclTime) {
				TRACE_LOG_FUNCTION(&lclTime, tid_, lvl_, _insert, __FILE__, line, function, 0, msg);
			}
		}
	}
}

#undef TRACE_CNTL
int64_t TRACE_CNTL(const char *_name, const char *_file, int nargs, const char *cmd, ...)
{
	int64_t retval;
	va_list ap;
	int arg_i[10];

	va_start(ap, cmd);

	if (strcmp(cmd,"printfd")==0) {
		int max=(int)(sizeof(arg_i)/sizeof(arg_i[0]));
		for (int ii=0; ii<((nargs<max)? nargs:max); ++ii) {
			arg_i[ii] = va_arg(ap, int);
		}
		retval = traceCntl( _name, _file, nargs, cmd,
		                   arg_i[0],arg_i[1],arg_i[2],arg_i[3],arg_i[4],arg_i[5],arg_i[6],arg_i[7],arg_i[8],arg_i[9]);
	} else {
		int max=(int)(sizeof(arg_i)/sizeof(arg_i[0]));
		for (int ii=0; ii<((nargs<max)? nargs:max); ++ii) {
			arg_i[ii] = va_arg(ap, int);
		}
		retval = traceCntl( _name, _file, nargs, cmd,
		                   arg_i[0],arg_i[1],arg_i[2],arg_i[3],arg_i[4],arg_i[5],arg_i[6],arg_i[7],arg_i[8],arg_i[9]);
	}
	va_end(ap);
	return retval;
}
