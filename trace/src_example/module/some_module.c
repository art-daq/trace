/*  This file (trace.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 26, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: some_module.c,v $
    rev="$Revision: 1.5 $$Date: 2014/04/17 19:40:18 $";
    */

// NOTE: this is trace_.c and not trace.c because nfs server has case
//       insensitive file system.

#include <linux/module.h>	// module_param, THIS_MODULE
#include <linux/init.h>		// module_init,_exit
#include <linux/kernel.h>	// KERN_INFO, printk
#include <linux/jiffies.h>
#include <linux/delay.h>	/* msleep */


#include "trace.h"

static struct workqueue_struct *wq=0; 
static int                      stop_requested=0;
static int			count=0;

static void my_work_function( struct work_struct *w );
static DECLARE_WORK(my_work, my_work_function);
static void my_work_function( struct work_struct *w )
{
    TRACE( 2,"hello from my_work_function jiffies: %ld", jiffies );
    msleep( 1500 );
    if (count < 20) count++;
    else if (count == 20)
    {   count++;
	TRACE_CNTL( "modeM", (uint64_t)0 );
    }
    if (!stop_requested)
	queue_work( wq, &my_work );    
}

 
static int __init init_some_module(void)
{
    int  ret=0;          /* SUCCESS */

    TRACE_CNTL( "name", "johnson" );
    TRACE_CNTL( "lvlmskM", (uint64_t)0xf );
    TRACE_CNTL( "lvlmskS", (uint64_t)1 );
    TRACE_CNTL( "modeS",   (uint64_t)1 );
    TRACE( 0, "init_some_module trace" );

    wq = create_singlethread_workqueue("mywork");
    if (wq)
	queue_work( wq, &my_work );

    return (ret);
}   // init_some_module

static void __exit exit_some_module(void)
{
    TRACE( 0, "exit_some_module() called" );

    if (wq)
    {   printk("requesting stop\n");
	stop_requested = 1;
	flush_workqueue(wq);
	TRACE( 0, "workqueue stopped/flushed" );
	destroy_workqueue(wq);
    }
}   // exit_some_module

module_init(init_some_module);
module_exit(exit_some_module);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("Third TRACE test");
MODULE_LICENSE("GPL"); /* Get rid of taint message by declaring code as GPL */
