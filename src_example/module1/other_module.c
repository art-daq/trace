/*  This file (trace.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 26, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: other_module.c,v $
    rev="$Revision: 491 $$Date: 2016-01-26 13:44:13 -0600 (Tue, 26 Jan 2016) $";
    */

// NOTE: this is trace_.c and not trace.c because nfs server has case
//       insensitive file system.

#include <linux/module.h>	// module_param, THIS_MODULE
#include <linux/init.h>		// module_init,_exit
#include <linux/kernel.h>	// KERN_INFO, printk
#include <linux/jiffies.h>
#include <linux/delay.h>	/* msleep */

#define TRACE_NAME "jones"  /* should, as of 2016.01.26, work */
#include "trace.h"

 
static int __init init_other_module(void)
{
    int  ret=0;          /* SUCCESS */

    printk(  KERN_INFO "init_other_module called\n" );

    TRACE( 0, "init_other_module trace" );

    return (ret);
}   // init_other_module

static void __exit exit_other_module(void)
{

    TRACE( 0, "exit_other_module() called" );
}   // exit_other_module

module_init(init_other_module);
module_exit(exit_other_module);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("Third TRACE test");
MODULE_LICENSE("GPL"); /* Get rid of taint message by declaring code as GPL */
