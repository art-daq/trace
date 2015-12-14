/*  This file (steve_module.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Apr 24, 2015. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: steve_module.c,v $
    rev="$Revision: 416 $$Date: 2015-10-13 11:48:10 -0500 (Tue, 13 Oct 2015) $";
    */

#include <linux/module.h>
#include <linux/version.h>      /* KERNEL_VERSION */
#include <linux/ftrace.h>
#include <linux/tracepoint.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
static int prt_num=0;
/* based on code in kernel/trace/trace_sched_switch.c */
static void my_trace_sched_switch_hook(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
				       void *__rq
# else
				       struct rq *__rq
# endif
				       , struct task_struct *prev
				       , struct task_struct *next )
{
	if (prt_num < 10)
	{   prt_num++;
	    printk("schedule: cpu=%d prev=%d next=%d\n"
		   , raw_smp_processor_id(),prev->pid,next->pid );
	}
}   // my_trace_sched_switch_hook

static void regfunc(struct tracepoint *tp, void *priv)
{
        int *ret = priv;
	if      (strcmp(tp->name,"sched_switch") == 0)
	{   printk("tracepoint: %s key.enabled=%d regfunc=%p %p %p\n"
		   , tp->name, tp->key.enabled.counter
		   , tp->regfunc
		   , tp->unregfunc
		   , tp->funcs
		   );
	    if (tp->funcs)
	    {   printk("  funcs[0].func=%p data=%p\n"
		       ,tp->funcs[0].func,tp->funcs[0].data);
	    }
	    *ret = tracepoint_probe_register( tp, my_trace_sched_switch_hook, NULL );
	}
	else if (strcmp(tp->name,"softirq_entry") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"softirq_exit") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"irq_handler_entry") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"irq_handler_exit") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"sys_enter") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"sys_exit") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
}
static void unregfunc(struct tracepoint *tp, void *ignore)
{
	if      (strcmp(tp->name,"sched_switch") == 0)
	{   printk("tracepoint: %s key.enabled=%d regfunc=%p %p %p\n"
		   , tp->name, tp->key.enabled.counter
		   , tp->regfunc
		   , tp->unregfunc
		   , tp->funcs
		   );
	    if (tp->funcs)
	    {   printk("  funcs[0].func=%p data=%p\n"
		       ,tp->funcs[0].func,tp->funcs[0].data);
	    }
	    tracepoint_probe_unregister( tp, my_trace_sched_switch_hook, NULL );
	}
	else if (strcmp(tp->name,"softirq_entry") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"softirq_exit") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"irq_handler_entry") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"irq_handler_exit") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"sys_enter") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
	else if (strcmp(tp->name,"sys_exit") == 0)
	{   printk("tracepoint: %s\n", tp->name);
	}
}
#endif

static int __init my_tp_init(void)
{
# if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
	int err=0;
	for_each_kernel_tracepoint(regfunc, &err);
	if (err)
	{   for_each_kernel_tracepoint(unregfunc, NULL);
	    return (err);
	}
	return 0;
# else
	return 0;
# endif
}

static void __exit my_tp_exit(void)
{
# if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
	for_each_kernel_tracepoint(unregfunc, NULL);
# endif
}

module_init(my_tp_init);
module_exit(my_tp_exit);

MODULE_AUTHOR("My name here");
MODULE_DESCRIPTION("Me!");
MODULE_LICENSE("GPL");
