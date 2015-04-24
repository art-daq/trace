/*  This file (steve_module.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Apr 24, 2015. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: steve_module.c,v $
    rev="$Revision: 1.1 $$Date: 2015-04-24 22:15:23 $";
    */

#include <linux/module.h>
#include <linux/version.h>      /* KERNEL_VERSION */
#include <linux/ftrace.h>
#include <linux/tracepoint.h>

static func(struct tracepoint *tp, void *ignore)
{
	printk("tracepoint: %s\n", tp->name);
}

static int __init my_tp_init(void)
{
# if LINUX_VERSION_CODE > KERNEL_VERSION(3, 15, 0)
	for_each_kernel_tracepoint(func, NULL);
# endif
	return 0;
}

static void __exit my_tp_exit(void)
{
}

module_init(my_tp_init);
module_exit(my_tp_exit);

MODULE_AUTHOR("My name here");
MODULE_DESCRIPTION("Me!");
MODULE_LICENSE("GPL");
