/*  This file (trace.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 26, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: some_module.c,v $
    rev="$Revision: 1.2 $$Date: 2014/02/04 00:39:29 $";
    */

// NOTE: this is trace_.c and not trace.c because nfs server has case
//       insensitive file system.

#include <linux/module.h>	// module_param, THIS_MODULE
#include <linux/init.h>		// module_init,_exit
#include <linux/kernel.h>	// KERN_INFO, printk
#include <linux/jiffies.h>
#include <linux/delay.h>	/* msleep */


#include "../module3/Trace_mmap4.h"

static struct workqueue_struct *wq=0; 
static int                      stop_requested=0;

static void my_work_function( struct work_struct *w );
static DECLARE_WORK(my_work, my_work_function);
static void my_work_function( struct work_struct *w )
{
    TRACE( 0,"hello from my_work_function" );
    msleep( 1500 );
    if (!stop_requested)
	queue_work( wq, &my_work );    
}

 
static int __init init_some_module(void)
{
    int  ret=0;          /* SUCCESS */

    printk(  KERN_INFO "init_some_module called\n" );

    TRACE( 2, "init_some_module trace" );

    wq = create_singlethread_workqueue("mywork");
    if (wq)
	queue_work( wq, &my_work );

    return (ret);
}   // init_some_module

static void __exit exit_some_module(void)
{
    printk(KERN_INFO "exit_some_module() called\n");

    if (wq)
    {   printk("requesting stop\n");
	stop_requested = 1;
	flush_workqueue(wq);
	printk("workqueue stopped/flushed\n");
	destroy_workqueue(wq);
    }
}   // exit_some_module

module_init(init_some_module);
module_exit(exit_some_module);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("Third TRACE test");
MODULE_LICENSE("GPL"); /* Get rid of taint message by declaring code as GPL */
