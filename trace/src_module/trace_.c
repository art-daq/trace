/*  This file (trace.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 26, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_.c,v $
    rev="$Revision: 1.1 $$Date: 2014/01/26 18:28:39 $";
    */

// NOTE: this is trace_.c and not trace.c because nfs server has case
//       insensitive file system.

#include <linux/module.h>	// module_param
#include <linux/init.h>		// module_init,_exit
//#include <linux/kernel.h>	// KERN_INFO

// ls /sys/module/TRACE/parameters
static int        entries;
module_param(     entries, int, 0444 );
MODULE_PARM_DESC( entries, "The number for entries in the circular buffer - default 128K" );
static int        fmtmax;
module_param(     fmtmax,  int, 0444 );
MODULE_PARM_DESC( fmtmax,  "Character beyond this length will be discarded" );
static int        argsmax;
module_param(     argsmax, int, 0444 );
MODULE_PARM_DESC( argsmax, "Maximum number of arguments that will be stored" );


static int  trace_proc_add( void )
{
    return (0);
}   // trace_proc_add
static void trace_proc_remove( void )
{
}   // trace_proc_remove

static int  trace_sched_switch_hook_add( void )
{
    return (0);
}   // trace_sched_switch_hook_add
static void trace_sched_switch_hook_remove( void )
{
}   // trace_sched_switch_hook_remove

 
static int __init init_trace_3(void)
{
    int ret=0;          /* SUCCESS */
    printk(KERN_INFO "init_trace_3() called\n");

    /* 1) create the buffer
       2) create a way to access it (/proc)
       3) register some traces
       4) setup userspace interrupt (sw)
    */
    if ((ret=trace_proc_add()))             return (ret);
    if ((ret=trace_sched_switch_hook_add()))goto undo1;
    return (ret);               /* success */
 undo1:
    trace_proc_remove();
    return (ret);
}   // init_trace_3

static void __exit exit_trace_3(void)
{

        printk(KERN_INFO "exit_trace_3() called\n");

        /* reverse the order:
           4) unregister userspace interrupt
           3) unregister some traces
           2) un-create a way to access the buffer (/proc)
           1) un-create the buffer
         */
        trace_sched_switch_hook_remove();
        trace_proc_remove();
}   // exit_trace_3

module_init(init_trace_3);
module_exit(exit_trace_3);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("Third TRACE");
MODULE_LICENSE("GPL"); /* Get rid of taint message by declaring code as GPL */
