/*  This file (trace.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 26, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_.c,v $
    rev="$Revision: 1.3 $$Date: 2014-01-31 04:25:52 $";
    */

// NOTE: this is trace_.c and not trace.c because nfs server has case
//       insensitive file system.

#include <linux/module.h>	// module_param, THIS_MODULE
#include <linux/init.h>		// module_init,_exit
#include <linux/kernel.h>	// KERN_INFO, printk
#include <linux/version.h>      /* KERNEL_VERSION */
#include <linux/mm.h>           /* do_mmap, vm_area_struct */
#include <linux/proc_fs.h>      /* create_proc_entry, struct proc_dir_entry */
#include <asm/io.h>             /* virt_to_phys */
#include <trace/events/sched.h> /* register/unregister_trace_sched_switch */


#include "Trace_mmap4.h"

struct traceControl_s  *traceControl_p=NULL;
struct traceEntryHdr_s *traceEntries_p;
struct traceNamLvls_s  *traceNamLvls_p=&traceNamLvls;
EXPORT_SYMBOL( traceControl_p );
EXPORT_SYMBOL( traceEntries_p );
EXPORT_SYMBOL( traceNamLvls_p );


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


static int trace_proc_buffer_mmap(  struct file              *file
				  , struct vm_area_struct    *vma )
{
    int           sts;
    void          *phys_addr;

    phys_addr = (void *)virt_to_phys( (char *)traceControl_p+vma->vm_pgoff );
    sts = io_remap_pfn_range(  vma, vma->vm_start
                             , (unsigned long)phys_addr >> PAGE_SHIFT
                             , vma->vm_end-vma->vm_start /* size */
                             , vma->vm_page_prot );
    if (sts) return -EAGAIN;
    return (0);
}   // trace_proc_buffer_mmap

static
struct file_operations trace_proc_buffer_file_operations = {
    owner:   THIS_MODULE,
    llseek:  NULL,           		/* lseek        */
    read:    NULL,   			/* read         */
    write:   NULL,           		/* write        */
    readdir: NULL,              	/* readdir      */
    poll:    NULL,              	/* poll         */
    ioctl:   NULL,         		/* ioctl        */
    mmap:    trace_proc_buffer_mmap,   	/* mmap         */
    NULL,                       	/* open         */
    NULL,                       	/* flush        */
    NULL,                       	/* release (close?)*/
    NULL,                       	/* fsync        */
    NULL,                       	/* fasync       */
    NULL,                       	/* check_media_change */
    NULL,                       	/* revalidate   */
    NULL                        	/* lock         */
};


static struct proc_dir_entry *trace_proc_root=NULL;

static int  trace_proc_add( int len )
{
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
    child = create_proc_entry( "buffer", S_IFREG|S_IRUGO, trace_proc_root );
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
    child->proc_fops = &trace_proc_buffer_file_operations;
    child->size = len;
    return (0);
}   // trace_proc_add

static void trace_proc_remove( void )
{
    printk( "trace module cleanup - removing /proc/trace directory tree\n" );
    remove_proc_entry( "buffer",  trace_proc_root );
    remove_proc_entry( "trace",   0 );
}   // trace_proc_remove



static void
trace_sched_switch_hook(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
                        void *__rq
# else
                        struct rq *__rq
# endif
                        , struct task_struct *prev
                        , struct task_struct *next )
{
        unsigned long flags;
        int cpu;
        int pc;

        pc = preempt_count(); /* what is this??? */

        local_irq_save(flags);

        cpu = raw_smp_processor_id();

	TRACE( 1, "sched: cpu=%d prev=%d next=%d", cpu, prev->pid, next->pid );

        local_irq_restore(flags);
}

static int  trace_sched_switch_hook_add( void )
{
    int err;
    err = register_trace_sched_switch( trace_sched_switch_hook
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
                                      , NULL
# endif
                                      );
    printk("trace_sched_switch_hook_init returning %d (0=success)\n", err );
    return (err);
}   // trace_sched_switch_hook_add
static void trace_sched_switch_hook_remove( void )
{
    unregister_trace_sched_switch( trace_sched_switch_hook
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
                                  , NULL
# endif
                                  );
}   // trace_sched_switch_hook_remove

 
static int __init init_trace_3(void)
{
    int  ret=0;          /* SUCCESS */
    int  len;
    int  num_namLvlTblEnts=200;
    int  num_params=10;
    int  siz_msg=128;
    int  num_entries=10000;
    int  siz_cntl_pages;

    len = traceMemLen( siz_msg, num_params, num_namLvlTblEnts, num_entries
		      , &siz_cntl_pages );

    printk(  KERN_INFO "init_trace_3 called -- attempt to allocate %d bytes\n"
	   , len );

    traceControl_p = (struct traceControl_s *)kmalloc( len, GFP_KERNEL );
    if (!traceControl_p) return -ENOMEM;
    printk("init_trace_3 kmalloc(%d)=%p\n",len,traceControl_p);

    traceControl_p->trace_initialized = 1;
    traceControl_p->num_namLvlTblEnts = num_namLvlTblEnts;
    traceControl_p->num_params       = num_params;
    traceControl_p->siz_msg          = siz_msg;
    traceControl_p->num_entries      = num_entries;
    traceControl_p->largest_multiple = (uint64_t)-1 - ((uint64_t)-1 % num_entries);
    traceControl_p->siz_entry        = entSiz( siz_msg, num_params );

    traceControl_p->wrIdxCnt         = 0;
    traceControl_p->trigActivePost   = 0;
    traceControl_p->mode.mode        = 0;

    traceNamLvls_p = (struct traceNamLvls_s *)			\
	((unsigned long)traceControl_p+siz_cntl_pages);

    traceEntries_p = (struct traceEntryHdr_s *)	\
	((unsigned long)traceNamLvls_p
	 +sizeof(struct traceNamLvls_s)*num_namLvlTblEnts);

    traceInitNames();

    traceTID = 0;

    traceControl_p->mode.s.M = 1;
    TRACE( 0, "kernel trace buffer initialized" );

    /* 1) create the buffer
       2) create a way to access it (/proc)
       3) register some traces
       4) setup userspace interrupt (sw)
    */
    if ((ret=trace_proc_add(len)))           return (ret);
    if ((ret=trace_sched_switch_hook_add())) goto undo1;
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

	if (traceControl_p)
	{   printk("exit_trace_3 kfree(%p)\n", traceControl_p );
	    kfree( traceControl_p );
	}
}   // exit_trace_3

module_init(init_trace_3);
module_exit(exit_trace_3);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("Third TRACE");
MODULE_LICENSE("GPL"); /* Get rid of taint message by declaring code as GPL */
