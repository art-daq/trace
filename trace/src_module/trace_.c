/*  This file (trace.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 26, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_.c,v $
    rev="$Revision: 1.42 $$Date: 2015-04-25 17:11:16 $";
    */

// NOTE: this is trace_.c and not trace.c because nfs server has case
//       insensitive file system.

#include <linux/module.h>	// module_param, THIS_MODULE
#include <linux/init.h>		// module_init,_exit
#include <linux/kernel.h>	// KERN_INFO, printk
#include <linux/version.h>      /* KERNEL_VERSION */
//#include <linux/interrupt.h>	// struct softirq_action  PROBLEM: many "redefined" and "conflicting type"
#include <linux/mm.h>           /* do_mmap, vm_area_struct */
#include <linux/io.h>		/* ioremap_page_range */
#include <linux/proc_fs.h>      /* create_proc_entry, struct proc_dir_entry */
#include <asm/io.h>             /* virt_to_phys */
#include <asm-generic/uaccess.h>/* copy_to_user */
#include <trace/events/sched.h> /* register/unregister_trace_sched_switch */
#include <trace/events/irq.h>	/*  */
#include <trace/events/syscalls.h>/* */
#define TRACE_IMPL		// implement traceInit
#include "trace.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
# define REGISTER_NULL_ARG    ,NULL
#else
# define REGISTER_NULL_ARG
#endif

struct traceControl_s  *traceControl_p=NULL;
struct traceEntryHdr_s *traceEntries_p;
struct traceNamLvls_s  *traceNamLvls_p;
EXPORT_SYMBOL_GPL( traceControl_p );
EXPORT_SYMBOL_GPL( traceEntries_p );
EXPORT_SYMBOL_GPL( traceNamLvls_p );
EXPORT_SYMBOL_GPL( trace_allow_printk );


// ls /sys/module/TRACE/parameters
module_param(     msgmax,  int, 0444 ); // defined in trace.h
MODULE_PARM_DESC( msgmax,  "Character beyond this length will be discarded" );

module_param(     argsmax, int, 0444 ); // defined in trace.h
MODULE_PARM_DESC( argsmax, "Maximum number of arguments that will be stored" );

module_param(     numents, int, 0444 ); // defined in trace.h
MODULE_PARM_DESC( numents, "The number for entries in the circular buffer" );

module_param(     namtblents, int, 0444 ); // defined in trace.h
MODULE_PARM_DESC( namtblents, "Number of name table entries" );

module_param(     trace_allow_printk, int, 0644 ); // defined in trace.h
MODULE_PARM_DESC( trace_allow_printk, "whether or not to allow TRACEs to do printk's" );


static int trace_proc_buffer_mmap(  struct file              *file
				  , struct vm_area_struct    *vma )
{
    int           sts;
    unsigned long pfn;
    off_t         off  =vma->vm_pgoff<<PAGE_SHIFT;
    unsigned long start=vma->vm_start;
    long 	  size =vma->vm_end-vma->vm_start;

    /* in order to force/assure read-only for a portion of a vma,
       I would have to use a function like mprotect_fixup which is
       not exported :( */
    while (size > 0)
    {
	pfn = vmalloc_to_pfn( ((char *)traceControl_p)+off );
	sts = io_remap_pfn_range(  vma, start,pfn,PAGE_SIZE,vma->vm_page_prot );
	if (sts) return -EAGAIN;
	start += PAGE_SIZE;
	off += PAGE_SIZE;
	size -= PAGE_SIZE;
    }

    return (0);
}   // trace_proc_buffer_mmap

/*static DEFINE_MUTEX(read_mutex);*/

static ssize_t trace_proc_buffer_read( struct file *fil, char __user *dst_p
				      , size_t siz, loff_t *off )
{
    long          must_check;
    long          lcl_off;
    long          dst_off =0;
    long          till_end;
    long          till_page;
    long          to_user;
    size_t        lcl_siz=siz;
    unsigned long vma;
    /*unsigned long kva;*/

    /*mutex_lock(&read_mutex);*/
    lcl_off  = (long)*off;
    till_end = (int)traceControl_p->memlen - lcl_off;
    while ((lcl_siz>0) && (till_end>0))
    {
	vma = (unsigned long)traceControl_p + lcl_off;
	till_page = PAGE_SIZE - (vma&(PAGE_SIZE-1));
	to_user = min3( till_page, till_end, (long)lcl_siz );
	/*kva = (unsigned long) page_address(vmalloc_to_page((void *)vma));
	printk("trace_proc_buffer_read: siz=%lu lclsiz=%lu lcloff=%ld "
	  "dstoff=%ld to_user=%ld tillend=%ld vma=0x%lx kva=0x%lx\n"
	  , siz, lcl_siz, lcl_off, dst_off, to_user, till_end, vma, kva );*/

	must_check = copy_to_user( dst_p+dst_off, (void*)vma/*kva*/, to_user );
	if (must_check != 0)
	    return (-EACCES);

	lcl_off += to_user;
	lcl_siz -= to_user;
	dst_off += to_user;
	till_end-= to_user;
    }
    /*printk("trace_proc_buffer_read - SUCCESS ret=%ld\n", siz-lcl_siz );*/
    *off += (siz-lcl_siz);
    /*mutex_unlock(&read_mutex);*/
    return (siz-lcl_siz);
}


static struct file_operations trace_proc_buffer_file_ops = {
    .owner=   THIS_MODULE,
    .llseek=  NULL,           		/* lseek        */
    .read=    trace_proc_buffer_read,	/* read         */
    .write=   NULL,           		/* write        */
    /*.readdir= NULL,             readdir -- comment out as unknown in 3.16.1 */
    .poll=    NULL,              	/* poll         */
# if 0
#  if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    .ioctl=   NULL,         		/* ioctl        */
#  else
    .unlocked_ioctl=NULL,         	/* ioctl        */
#  endif
# endif
    .mmap=    trace_proc_buffer_mmap,   /* mmap         */
    .open=    NULL/*trace_proc_buffer_open generic_file_open*/,   /* open     */
    NULL,                       	/* flush        */
    .release= NULL/*trace_proc_buffer_release*/,/* release (close) */
    NULL,                       	/* fsync        */
    NULL,                       	/* fasync       */
    NULL,                       	/* check_media_change */
    NULL,                       	/* revalidate   */
    NULL                        	/* lock         */
};


static struct proc_dir_entry *trace_proc_root=NULL;

static int  trace_proc_add( int len )
{

# if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    struct proc_dir_entry *child;
    /*  Note: struct proc_dir_entry (include/linux/proc_fs.h) has pointer
        to struct inode_operations which has pointer to struct
        file_operations (see include/linux/fs.h). This will allow for mmap
        operation.
    */

    trace_proc_root = create_proc_entry("trace", S_IFDIR, 0/*parent is /proc*/);
    if (!trace_proc_root)
    {   printk( "proc_trace_create: error creating proc_entry\n" );
        return (1);
    }

    /* CREATE BUFFER FILE */
    child = create_proc_entry( "buffer", S_IFREG|S_IRUGO|S_IWUGO, trace_proc_root );
    if (!child)
    {   remove_proc_entry( "buffer",  trace_proc_root );
	printk( "proc_trace_create: error creating buffer file\n" );
        return (1);
    }
    /*  override defaults from
        fs/proc/generic.c:struct inode_operations proc_file_inode_operations
        set in fs/proc/root.c:proc_register
    */
    //child->proc_iops = &trace_proc_buffer_inode_operations;
    child->proc_fops = &trace_proc_buffer_file_ops;
    child->size = len;
# else
    if (!proc_mkdir("trace", NULL))
    {   printk( "proc_trace_create: error creating proc_entry\n" );
        return (1);
    }

    trace_proc_root = proc_create("trace/buffer",0666,NULL,&trace_proc_buffer_file_ops);
    if (trace_proc_root == NULL)
    {	printk( "proc_trace_create: error creating proc_entry\n" );
        return (1);
    }
    proc_set_size( trace_proc_root, len );
# endif
    return (0);
}   // trace_proc_add

static void trace_proc_remove( void )
{
    printk( "trace module cleanup - removing /proc/trace directory tree\n" );
# if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    remove_proc_entry( "buffer",  trace_proc_root );
# else
    remove_proc_entry( "trace/buffer",  0 );
# endif
    remove_proc_entry( "trace",   0 );
}   // trace_proc_remove


// =========================================================================

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
	TRACE( 31, "schedule: cpu=%d prev=%d next=%d", raw_smp_processor_id(), prev->pid, next->pid );
}   // my_trace_sched_switch_hook

static void my_trace_hirq_enter(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
				void *new,
# endif
				int irq
				, struct irqaction *action )
{
        // comment out since I don't use action ptr. unsigned long flags;
        //local_irq_save(flags);
	TRACE( 29, "hirqenter: cpu=%d irq=%d"
	      , raw_smp_processor_id(), irq );
        //local_irq_restore(flags);
}   // my_trace_irq_enter

static void my_trace_hirq_exit(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
			       void *new,
# endif
			       int irq,
			       struct irqaction *action, int ret )
{
	// comment out since I don't use action ptr. unsigned long flags;
	//local_irq_save(flags);
	TRACE( 30, "hirqexit: cpu=%d irq=%d ret=%d"
	      , raw_smp_processor_id(), irq, ret );
        //local_irq_restore(flags);
}   // my_trace_irq_exit

struct softirq_action { void (*action)(struct softirq_action *); };
static void my_trace_sirq_enter(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
				void *new,
				unsigned int vec_nr )
{
# else
				struct softirq_action *x,
				struct softirq_action *y )
{	unsigned int vec_nr = x - y;
# endif
	TRACE( 27,"sirqenter: cpu=%d vec_nr=%u",raw_smp_processor_id(),vec_nr );
}   // my_trace_sirq_enter

static void my_trace_sirq_exit(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
			       void *new,
				unsigned int vec_nr )
{
# else
			       struct softirq_action *x,
			       struct softirq_action *y )
{	unsigned int vec_nr = x - y;
# endif
	TRACE( 28,"sirqexit: cpu=%d vec_nr=%u",raw_smp_processor_id(),vec_nr );
}   // my_trace_sirq_exit

// Ref. kernel/trace/trace_syscalls.c:void ftrace_syscall_enter(void *ignore, struct pt_regs *regs, long id)
static void my_trace_sys_enter(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
			       void *ignore,
# endif
			       struct pt_regs *regs, long id )
{
        int syscall_nr;

        syscall_nr = syscall_get_nr(current, regs);
        if (syscall_nr < 0)
                return;

	TRACE( 25, "sysenter: cpu=%d syscall=%d id=%ld", raw_smp_processor_id(), syscall_nr, id );

}   // my_trace_sys_enter

static void my_trace_sys_exit(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
			       void *ignore,
# endif
			       struct pt_regs *regs, long ret )
{
        int syscall_nr;
	long syscall_ret;

        syscall_nr = syscall_get_nr(current, regs);
        if (syscall_nr < 0)
                return;
	syscall_ret=syscall_get_return_value(current, regs);
	if (syscall_ret == ret)
	    TRACE( 26, "sys_exit: cpu=%d syscall=%d ret=0x%lx (%ld)"
		  , raw_smp_processor_id(), syscall_nr, ret, ret );
	else
	    TRACE( 26, "sys_exit: cpu=%d syscall=%d ret=0x%lx (%ld), 0x%lx (%ld)"
		  , raw_smp_processor_id(), syscall_nr
		  , syscall_ret, syscall_ret, ret, ret );

}   // my_trace_sys_exit


// ---------------------------------------------------------------------------
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
static void regfunc(struct tracepoint *tp, void *priv)
{
        int *ret = priv;
	*ret=0;
	if      (strcmp(tp->name,"sched_switch") == 0)
	    *ret = tracepoint_probe_register( tp, my_trace_sched_switch_hook, NULL );
	else if (strcmp(tp->name,"irq_handler_entry") == 0)
	    *ret = tracepoint_probe_register( tp, my_trace_hirq_enter, NULL );
	else if (strcmp(tp->name,"irq_handler_exit") == 0)
	    *ret = tracepoint_probe_register( tp, my_trace_hirq_exit, NULL );
	else if (strcmp(tp->name,"softirq_entry") == 0)
	    *ret = tracepoint_probe_register( tp, my_trace_sirq_enter, NULL );
	else if (strcmp(tp->name,"softirq_exit") == 0)
	    *ret = tracepoint_probe_register( tp, my_trace_sirq_exit, NULL );
	else if (strcmp(tp->name,"sys_enter") == 0)
	    *ret = tracepoint_probe_register( tp, my_trace_sys_enter, NULL );
	else if (strcmp(tp->name,"sys_exit") == 0)
	    *ret = tracepoint_probe_register( tp, my_trace_sys_exit, NULL );
}
static void unregfunc(struct tracepoint *tp, void *ignore)
{
	if      (strcmp(tp->name,"sched_switch") == 0)
	    tracepoint_probe_unregister( tp, my_trace_sched_switch_hook, NULL );
	else if (strcmp(tp->name,"irq_handler_entry") == 0)
	    tracepoint_probe_unregister( tp, my_trace_hirq_enter, NULL );
	else if (strcmp(tp->name,"irq_handler_exit") == 0)
	    tracepoint_probe_unregister( tp, my_trace_hirq_exit, NULL );
	else if (strcmp(tp->name,"softirq_entry") == 0)
	    tracepoint_probe_unregister( tp, my_trace_sirq_enter, NULL );
	else if (strcmp(tp->name,"softirq_exit") == 0)
	    tracepoint_probe_unregister( tp, my_trace_sirq_exit, NULL );
	else if (strcmp(tp->name,"sys_enter") == 0)
	    tracepoint_probe_unregister( tp, my_trace_sys_enter, NULL );
	else if (strcmp(tp->name,"sys_exit") == 0)
	    tracepoint_probe_unregister( tp, my_trace_sys_exit, NULL );
}
#endif

static int  trace_sched_switch_hook_add( void )
{
    int err=0;
# if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
	for_each_kernel_tracepoint(regfunc, &err);
# else
    err = register_trace_sched_switch( my_trace_sched_switch_hook 
                                      REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sched returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_irq_handler_entry( my_trace_hirq_enter REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: hirq_entry returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_irq_handler_exit( my_trace_hirq_exit REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: hirq_exit returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_softirq_entry( my_trace_sirq_enter REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sirq_entry returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_softirq_exit( my_trace_sirq_exit REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sirq_exit returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_sys_enter( my_trace_sys_enter REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sys_enter returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_sys_exit( my_trace_sys_exit REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sys_exit returning %d (0=success)\n", err );
# endif
    return (err);
}   // trace_sched_switch_hook_add

static void trace_sched_switch_hook_remove( void )
{
# if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
	for_each_kernel_tracepoint(unregfunc, NULL);
# else
    unregister_trace_sys_exit( my_trace_sys_exit REGISTER_NULL_ARG );
    unregister_trace_sys_enter( my_trace_sys_enter REGISTER_NULL_ARG );
    unregister_trace_softirq_exit( my_trace_sirq_exit REGISTER_NULL_ARG );
    unregister_trace_softirq_entry( my_trace_sirq_enter REGISTER_NULL_ARG );
    unregister_trace_irq_handler_exit( my_trace_hirq_exit REGISTER_NULL_ARG );
    unregister_trace_irq_handler_entry( my_trace_hirq_enter REGISTER_NULL_ARG );
    unregister_trace_sched_switch( my_trace_sched_switch_hook
				   REGISTER_NULL_ARG );
# endif
}   // trace_sched_switch_hook_remove


 
static int __init init_trace_3(void)
{
    int  ret=0;          /* SUCCESS */

    if ((ret=traceInit(NULL)) != 0) return (ret);

    traceTID = 0;

    traceControl_p->mode.bits.M = 1;
    TRACE( 0, "kernel trace buffer initialized" );

    /* 1) create the buffer
       2) create a way to access it (/proc)
       3) register some traces
       4) setup userspace interrupt (sw)
    */
    if ((ret=trace_proc_add(traceControl_p->memlen))) goto undo1;
    if ((ret=trace_sched_switch_hook_add()))          goto undo2;
    return (ret);               /* success */
 undo2:
    trace_proc_remove();
 undo1:
    vfree( traceControl_p );
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

	if (traceControl_p)
	{   printk("exit_trace_3 vfree(%p)\n", traceControl_p );
	    vfree( traceControl_p );
	}
}   // exit_trace_3

module_init(init_trace_3);
module_exit(exit_trace_3);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("Third TRACE");
MODULE_LICENSE("GPL"); /* It is for anyone/everyone, I don't care as long as it works for me, and besides I won't want to taint the kernel */
