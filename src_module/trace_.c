/*  This file (trace.c) was created by Ron Rechenmacher <ron@fnal.gov> on
    Jan 26, 2014. "TERMS AND CONDITIONS" governing this file are in the README
    or COPYING file. If you do not have such a file, one can be obtained by
    contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
    $RCSfile: trace_.c,v $
    rev="$Revision: 1595 $$Date: 2023-03-23 15:47:57 -0500 (Thu, 23 Mar 2023) $";
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
#include <linux/uaccess.h>      /* copy_to_user */
#include <asm/uaccess.h>        /* copy_to_user */
#include <trace/events/sched.h> /* register/unregister_trace_sched_switch */
#include <trace/events/irq.h>	/*  */
#include <trace/events/syscalls.h>/* */
#define TRACE_IMPL		// implement traceInit
#define TRACE_DEFINE
#include "TRACE/trace.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
# define REGISTER_NULL_ARG    ,NULL
#else
# define REGISTER_NULL_ARG
#endif

#ifdef MODULE
// ls /sys/module/TRACE/parameters
module_param(     msgmax,  int, 0444 ); // defined in trace.h
MODULE_PARM_DESC( msgmax,  "Character beyond this length will be discarded" );
module_param(     argsmax, int, 0444 ); // defined in trace.h
MODULE_PARM_DESC( argsmax, "Maximum number of arguments that will be stored" );
module_param(     numents, int, 0444 ); // defined in trace.h
MODULE_PARM_DESC( numents, "The number for entries in the circular buffer" );
module_param(     namtblents, int, 0444 ); // defined in trace.h
MODULE_PARM_DESC( namtblents, "Number of name table entries" );
module_param(     trace_buffer_numa_node, int, 0444 ); // defined in trace.h
MODULE_PARM_DESC( trace_buffer_numa_node, "Numa node for trace buffer kernel memory" );
module_param(     trace_allow_printk, int, 0644 ); // defined in trace.h
MODULE_PARM_DESC( trace_allow_printk, "whether or not to allow TRACEs to do printk's" );
module_param_string(trace_print, trace_print, sizeof(trace_print), 0644 ); // defined in trace.h
MODULE_PARM_DESC( trace_print, "printk print control - default: \"%T %n %L %M\"" );
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,1)
module_param(     trace_lvlS, ullong, 0644 ); // defined in trace.h
MODULE_PARM_DESC( trace_lvlS, "default printk lvl" );
module_param(     trace_lvlM, ullong, 0644 ); // defined in trace.h
MODULE_PARM_DESC( trace_lvlM, "default memory lvl" );
#  endif
#else      /* MODULE */
# define KSTRVAL( str, parm, xx,fmt )				\
	unsigned long val;\
	if (kstrtoul(str, 0, &val)) {\
		pr_warn("invalid " # parm " parameter '%s'\n", str);\
		return 0;\
	}\
	xx = val;\
	pr_info("setting " # parm " to %" # fmt "\n", xx);\
	return 1

static int __init trace_msgmax_setup(char *str)
{	KSTRVAL( str, trace_msgmax, msgmax, d );
}
__setup("trace_msgmax=", trace_msgmax_setup);
static int __init trace_argsmax_setup(char *str)
{	KSTRVAL( str, trace_argsmax, argsmax, d );
}
__setup("trace_argsmax=", trace_argsmax_setup);
static int __init trace_numents_setup(char *str)
{	KSTRVAL( str, trace_numents, numents, d );
}
__setup("trace_numents=", trace_numents_setup);
static int __init trace_namtblents_setup(char *str)
{	KSTRVAL( str, trace_namtblents, namtblents, d );
}
__setup("trace_namtblents=", trace_namtblents_setup);
static int __init trace_buffer_numa_node_setup(char *str)
{	KSTRVAL( str, trace_buffer_numa_node, trace_buffer_numa_node, d );
}
__setup("trace_buffer_numa_node=", trace_buffer_numa_node_setup);
static int __init trace_allow_printk_setup(char *str)
{	KSTRVAL( str, trace_allow_printk, trace_allow_printk, d );
}
__setup("trace_allow_printk=", trace_allow_printk_setup);
static int __init trace_lvlS_setup(char *str)
{	KSTRVAL( str, trace_lvlS, trace_lvlS, llu );
}
__setup("trace_lvlS=", trace_lvlS_setup);
static int __init trace_lvlM_setup(char *str)
{	KSTRVAL( str, trace_lvlM, trace_lvlM, llu );
}
__setup("trace_lvlM=", trace_lvlM_setup);

static int __init trace_print_setup(char *str)
{	strncpy(trace_print,str,sizeof(trace_print));
	trace_print[sizeof(trace_print)-1]='\0';
	pr_info("setting trace_print to %s\n", trace_print);
	return 1;
}
__setup("trace_print=", trace_print_setup);

static ssize_t trace_proc_control_write( struct file *fil, const char __user *src_p
				      , size_t siz, loff_t *off )
{
        char    kernelBuffer[1024];
        char    *sptr;
        int     cc;
	cc = (siz<(sizeof(kernelBuffer)-1))? siz: (sizeof(kernelBuffer)-1);
    if (copy_from_user(  kernelBuffer, src_p, cc ) != 0)
		return -1;
    kernelBuffer[cc] = '\0'; /* terminate our copy of the string */
    for (sptr=kernelBuffer; *sptr; sptr++)
    {
        if      (strncmp("trace_allow_printk=",sptr,sizeof("trace_allow_printk=")-1)==0)
        {   sptr+=sizeof("trace_allow_printk=")-1;
            trace_allow_printk = simple_strtoul(sptr,&sptr,0);
        }
		else if (strncmp("trace_lvlS=",sptr,sizeof("trace_lvlS=")-1)==0)
        {   sptr+=sizeof("trace_lvlS=")-1;
            trace_lvlS = simple_strtoul(sptr,&sptr,0);
        }
		else if (strncmp("trace_lvlM=",sptr,sizeof("trace_lvlM=")-1)==0)
        {   sptr+=sizeof("trace_lvlM=")-1;
            trace_lvlM = simple_strtoul(sptr,&sptr,0);
        }
		else if (strncmp("trace_print=",sptr,sizeof("trace_print=")-1)==0)
        {   sptr+=sizeof("trace_print=")-1;
			strncpy(trace_print,sptr,sizeof(trace_print));
			trace_print[sizeof(trace_print)-1]='\0';
		}
	}
	return siz;
}
static ssize_t trace_proc_control_read( struct file *fil, char __user *dst_p
				      , size_t siz, loff_t *off )
{
	char    kernelBuffer[1024];
	int     cc;
	cc  = snprintf( &(kernelBuffer[0]), siz,"trace_allow_printk=%d\n", trace_allow_printk );
	if (*off >= cc) return 0;
	cc  = snprintf( &(kernelBuffer[0]), siz,"trace_print=%s\n", trace_print );
	if (*off >= cc) return 0;
	cc += snprintf( &(kernelBuffer[cc]),siz,"trace_lvlS=0x%llx\n", trace_lvlS );
	if (*off >= cc) return 0;
	cc += snprintf( &(kernelBuffer[cc]),siz,"trace_lvlM=0x%llx\n", trace_lvlM );
	if (*off >= cc) return 0;
	if (copy_to_user( dst_p, &kernelBuffer[*off], cc-*off ) != 0)
		return -1;
	cc = cc-*off;
	*off += cc;
	return cc;
}
static struct file_operations trace_proc_control_file_ops = {
    .owner=   THIS_MODULE,
    .write=   trace_proc_control_write,	   /* write        */
	.read=    trace_proc_control_read,	   /* read         */
};
#endif	/* else MODULE */

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


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static struct proc_ops        trace_proc_buffer_file_ops = {
	.proc_read=trace_proc_buffer_read,	/* read         */
	.proc_mmap=trace_proc_buffer_mmap,   /* mmap         */
};
#else
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
# endif   /* 0 */
    .mmap=    trace_proc_buffer_mmap,   /* mmap         */
    .open=    NULL/*trace_proc_buffer_open generic_file_open*/,   /* open     */
    .flush=   NULL,                       	/* flush        */
    .release= NULL/*trace_proc_buffer_release*/,/* release (close) */
    .fsync=   NULL                       	/* fsync        */
    /* the rest will be NULL */	/* fasync       */
                           	/* check_media_change */
                           	/* revalidate   */
                           	/* lock         */
};
#endif    /* else LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0) */


static struct proc_dir_entry *trace_proc_root=NULL;

//static
int  trace_3_proc_add( int len )
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
    printk("trace_.c:trace_proc_add start\n");
    if (!proc_mkdir("trace", NULL))
    {   printk( "proc_trace_create: error creating proc_entry\n" );
        return (1);
    }
    printk("trace_.c:trace_proc_add proc_mkdir(\"trace\", NULL)=OK\n");

    trace_proc_root = proc_create("trace/buffer",0666,NULL,&trace_proc_buffer_file_ops);
    if (trace_proc_root == NULL)
    {	printk( "proc_trace_create: error creating buffer proc_entry\n" );
        return (1);
    }
    proc_set_size( trace_proc_root, len );
#  ifndef MODULE
    if (proc_create("trace/control",0666,NULL,&trace_proc_control_file_ops) == NULL)
    {	printk( "proc_trace_create: error creating control proc_entry\n" );
        return (1);
    }
#  endif
# endif      /* else LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0) */
    return (0);
}   // trace_proc_add

# ifdef MODULE
static void trace_proc_remove( void )
{
    printk( "trace module cleanup - removing /proc/trace directory tree\n" );
#  if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 0)
    remove_proc_entry( "buffer",  trace_proc_root );
#  else
    remove_proc_entry( "trace/buffer",  0 );
#  endif
    remove_proc_entry( "trace",   0 );
}   // trace_proc_remove
# endif   /* MODULE */

// =========================================================================

/* based on code in kernel/trace/trace_sched_switch.c */
static void _sched_switch_hook(
# if   LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
									   void *ignore, bool preempt
# elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
				       void *__rq
# else
				       struct rq *__rq
# endif
				       , struct task_struct *prev
				       , struct task_struct *next )
{
	TRACE( 31, "cpu=%d prev=%d next=%d", raw_smp_processor_id(), prev->pid, next->pid );
	//TRACE( 31, "schedule: cpu=%d prev=%d next=%d", raw_smp_processor_id(), task_pid_nr(prev), task_pid_nr(next) );
	//TRACE( 31, "schedule: cpu=%d prev=%p next=%p", raw_smp_processor_id(), prev, next );
}   // _sched_switch_hook

static void _hirq_enter(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
				void *new,
# endif
				int irq
				, struct irqaction *action )
{
        // comment out since I don't use action ptr. unsigned long flags;
        //local_irq_save(flags);
	TRACE( 29, "cpu=%d irq=%d"
	      , raw_smp_processor_id(), irq );
        //local_irq_restore(flags);
}   // my_trace_irq_enter

static void _hirq_exit(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
			       void *new,
# endif
			       int irq,
			       struct irqaction *action, int ret )
{
	// comment out since I don't use action ptr. unsigned long flags;
	//local_irq_save(flags);
	TRACE( 30, "cpu=%d irq=%d ret=%d"
	      , raw_smp_processor_id(), irq, ret );
        //local_irq_restore(flags);
}   // my_trace_irq_exit

struct softirq_action { void (*action)(struct softirq_action *); };
static void _sirq_enter(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
				void *new,
				unsigned int vec_nr )
{
# else
				struct softirq_action *x,
				struct softirq_action *y )
{	unsigned int vec_nr = x - y;
# endif
	TRACE( 27,"cpu=%d vec_nr=%u",raw_smp_processor_id(),vec_nr );
}   // _sirq_enter

static void _sirq_exit(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
			       void *new,
				unsigned int vec_nr )
{
# else
			       struct softirq_action *x,
			       struct softirq_action *y )
{	unsigned int vec_nr = x - y;
# endif
	TRACE( 28,"cpu=%d vec_nr=%u",raw_smp_processor_id(),vec_nr );
}   // _sirq_exit

// Ref. kernel/trace/trace_syscalls.c:void ftrace_syscall_enter(void *ignore, struct pt_regs *regs, long id)
static void _sys_enter(
# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
			       void *ignore,
# endif
			       struct pt_regs *regs, long id )
{
        int syscall_nr;

        syscall_nr = syscall_get_nr(current, regs);
        if (syscall_nr < 0)
                return;

	if (syscall_nr == id)
	    TRACE( 25, "cpu=%d syscall=%ld", raw_smp_processor_id(), id );
	else
	    TRACE( 25, "cpu=%d syscall=%d id=%ld", raw_smp_processor_id(), syscall_nr, id );

}   // _sys_enter

static void _sys_exit(
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
	    TRACE( 26, "cpu=%d syscall=%d ret=0x%lx (%ld)"
		  , raw_smp_processor_id(), syscall_nr, ret, ret );
	else
	    TRACE( 26, "cpu=%d syscall=%d ret=0x%lx (%ld), 0x%lx (%ld)"
		  , raw_smp_processor_id(), syscall_nr
		  , syscall_ret, syscall_ret, ret, ret );

}   // _sys_exit


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)

static void _rcu_utilization(  void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 32, "cpu=%d rcu_utilization", raw_smp_processor_id());
}

static void _rcu_stall_warning(void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 33, "cpu=%d rcu_stall_warning", raw_smp_processor_id());
}

static void _alarmtimer_fired( void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 34, "cpu=%d alarmtimer_fired", raw_smp_processor_id());
}

static void _nmi_handler(      void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 35, "cpu=%d nmi_handler", raw_smp_processor_id());
}

static void _mm_lru_insertion( void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 36, "cpu=%d mm_lru_insertion", raw_smp_processor_id());
}

static void _mm_lru_activate(  void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 37, "cpu=%d mm_lru_activate", raw_smp_processor_id());
}

static void _powernv_throttle( void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 38, "cpu=%d powernv_throttle", raw_smp_processor_id());
}

static void _cpu_frequency(    void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 39, "cpu=%d cpu_frequency", raw_smp_processor_id());
}

static void _cpu_frequency_limits(void *ignore,
			          struct pt_regs *regs, long ret )
{
	TRACE( 40, "cpu=%d cpu_frequency_limits", raw_smp_processor_id());
}

static void _cpu_migrate_begin(void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 41, "cpu=%d cpu_migrate_begin", raw_smp_processor_id());
}

static void _mm_migrate_pages( void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 42, "cpu=%d mm_migrate_pages", raw_smp_processor_id());
}

static void _ipi_entry(        void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 43, "cpu=%d ipi_entry", raw_smp_processor_id());
}

static void _ipi_exit(         void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 44, "cpu=%d ipi_exit", raw_smp_processor_id());
}

static void _timer_expire_entry(void *ignore,
			        struct pt_regs *regs, long ret )
{
	TRACE( 45, "cpu=%d timer_expire_entry", raw_smp_processor_id());
}

static void _timer_expire_exit(void *ignore,
			       struct pt_regs *regs, long ret )
{
	TRACE( 46, "cpu=%d timer_expire_exit", raw_smp_processor_id());
}

static void _hrtimer_expire_entry(void *ignore,
			          struct pt_regs *regs, long ret )
{
	TRACE( 47, "cpu=%d hrtimer_expire_entry", raw_smp_processor_id());
}

static void _hrtimer_expire_exit(void *ignore,
			         struct pt_regs *regs, long ret )
{
	TRACE( 48, "cpu=%d hrtimer_expire_exit", raw_smp_processor_id());
}

#endif     /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0) */



// ---------------------------------------------------------------------------
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
static void regfunc(struct tracepoint *tp, void *priv)
{
        int *ret = priv;
	*ret=0;
	if      (strcmp(tp->name,"sched_switch") == 0) {
	    *ret = tracepoint_probe_register( tp, _sched_switch_hook, NULL );
		printk("TRACE tracepoint_probe_register sched_switch returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"irq_handler_entry") == 0) {
	    *ret = tracepoint_probe_register( tp, _hirq_enter, NULL );
		printk("TRACE tracepoint_probe_register irq_handler_entry returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"irq_handler_exit") == 0) {
	    *ret = tracepoint_probe_register( tp, _hirq_exit, NULL );
		printk("TRACE tracepoint_probe_register irq_handler_exit returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"softirq_entry") == 0) {
	    *ret = tracepoint_probe_register( tp, _sirq_enter, NULL );
		printk("TRACE tracepoint_probe_register softirq_entry returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"softirq_exit") == 0) {
	    *ret = tracepoint_probe_register( tp, _sirq_exit, NULL );
		printk("TRACE tracepoint_probe_register softirq_exit returned %d\n", *ret );
	}
    /* for some reason, registering these early in boot (at the end of
	   init/main.c:start_kernel) causes the syscall tracing to stop working
	   (doesn't do any syscall tracing even when apparently enabled).
	   Solution - call later - at the beginning of init/main.c:kernel_init */
	else if (strcmp(tp->name,"sys_enter") == 0) {
	    *ret = tracepoint_probe_register( tp, _sys_enter, NULL );
		printk("TRACE tracepoint_probe_register sys_enter returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"sys_exit") == 0) {
	    *ret = tracepoint_probe_register( tp, _sys_exit, NULL );
		printk("TRACE tracepoint_probe_register sys_exit returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"rcu_utilization") == 0) {
	    *ret = tracepoint_probe_register( tp, _rcu_utilization, NULL );
		printk("TRACE tracepoint_probe_register rcu_utilization returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"rcu_stall_warning") == 0) {
	    *ret = tracepoint_probe_register( tp, _rcu_stall_warning, NULL );
		printk("TRACE tracepoint_probe_register rcu_stall_warning returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"alarmtimer_fired") == 0) {
	    *ret = tracepoint_probe_register( tp, _alarmtimer_fired, NULL );
		printk("TRACE tracepoint_probe_register alarmtimer_fired returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"nmi_handler") == 0) {
	    *ret = tracepoint_probe_register( tp, _nmi_handler, NULL );
		printk("TRACE tracepoint_probe_register nmi_handler returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"mm_lru_insertion") == 0) {
	    *ret = tracepoint_probe_register( tp, _mm_lru_insertion, NULL );
		printk("TRACE tracepoint_probe_register mm_lru_insertion returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"mm_lru_activate") == 0) {
	    *ret = tracepoint_probe_register( tp, _mm_lru_activate, NULL );
		printk("TRACE tracepoint_probe_register mm_lru_activate returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"powernv_throttle") == 0) {
	    *ret = tracepoint_probe_register( tp, _powernv_throttle, NULL );
		printk("TRACE tracepoint_probe_register powernv_throttle returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"cpu_frequency") == 0) {
	    *ret = tracepoint_probe_register( tp, _cpu_frequency, NULL );
		printk("TRACE tracepoint_probe_register cpu_frequency returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"cpu_frequency_limits") == 0) {
	    *ret = tracepoint_probe_register( tp, _cpu_frequency_limits, NULL );
		printk("TRACE tracepoint_probe_register cpu_frequency_limits returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"cpu_migrate_begin") == 0) {
	    *ret = tracepoint_probe_register( tp, _cpu_migrate_begin, NULL );
		printk("TRACE tracepoint_probe_register cpu_migrate_begin returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"mm_migrate_pages") == 0) {
	    *ret = tracepoint_probe_register( tp, _mm_migrate_pages, NULL );
		printk("TRACE tracepoint_probe_register mm_migrate_pages returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"ipi_entry") == 0) {
	    *ret = tracepoint_probe_register( tp, _ipi_entry, NULL );
		printk("TRACE tracepoint_probe_register ipi_entry returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"ipi_exit") == 0) {
	    *ret = tracepoint_probe_register( tp, _ipi_exit, NULL );
		printk("TRACE tracepoint_probe_register ipi_exit returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"timer_expire_entry") == 0) {
	    *ret = tracepoint_probe_register( tp, _timer_expire_entry, NULL );
		printk("TRACE tracepoint_probe_register timer_expire_entry returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"timer_expire_exit") == 0) {
	    *ret = tracepoint_probe_register( tp, _timer_expire_exit, NULL );
		printk("TRACE tracepoint_probe_register timer_expire_exit returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"hrtimer_expire_entry") == 0) {
	    *ret = tracepoint_probe_register( tp, _hrtimer_expire_entry, NULL );
		printk("TRACE tracepoint_probe_register hrtimer_expire_entry returned %d\n", *ret );
	}
	else if (strcmp(tp->name,"hrtimer_expire_exit") == 0) {
	    *ret = tracepoint_probe_register( tp, _hrtimer_expire_exit, NULL );
		printk("TRACE tracepoint_probe_register hrtimer_expire_exit returned %d\n", *ret );
	}
}
# ifdef MODULE
static void unregfunc(struct tracepoint *tp, void *ignore)
{
	if      (strcmp(tp->name,"sched_switch") == 0)
	    tracepoint_probe_unregister( tp, _sched_switch_hook, NULL );
	else if (strcmp(tp->name,"irq_handler_entry") == 0)
	    tracepoint_probe_unregister( tp, _hirq_enter, NULL );
	else if (strcmp(tp->name,"irq_handler_exit") == 0)
	    tracepoint_probe_unregister( tp, _hirq_exit, NULL );
	else if (strcmp(tp->name,"softirq_entry") == 0)
	    tracepoint_probe_unregister( tp, _sirq_enter, NULL );
	else if (strcmp(tp->name,"softirq_exit") == 0)
	    tracepoint_probe_unregister( tp, _sirq_exit, NULL );
	else if (strcmp(tp->name,"sys_enter") == 0)
	    tracepoint_probe_unregister( tp, _sys_enter, NULL );
	else if (strcmp(tp->name,"sys_exit") == 0)
	    tracepoint_probe_unregister( tp, _sys_exit, NULL );
	else if (strcmp(tp->name,"rcu_utilization") == 0)
	    tracepoint_probe_unregister( tp, _rcu_utilization, NULL );
	else if (strcmp(tp->name,"rcu_stall_warning") == 0)
	    tracepoint_probe_unregister( tp, _rcu_stall_warning, NULL );
	else if (strcmp(tp->name,"alarmtimer_fired") == 0)
	    tracepoint_probe_unregister( tp, _alarmtimer_fired, NULL );
	else if (strcmp(tp->name,"nmi_handler") == 0)
	    tracepoint_probe_unregister( tp, _nmi_handler, NULL );
	else if (strcmp(tp->name,"mm_lru_insertion") == 0)
	    tracepoint_probe_unregister( tp, _mm_lru_insertion, NULL );
	else if (strcmp(tp->name,"mm_lru_activate") == 0)
	    tracepoint_probe_unregister( tp, _mm_lru_activate, NULL );
	else if (strcmp(tp->name,"powernv_throttle") == 0)
	    tracepoint_probe_unregister( tp, _powernv_throttle, NULL );
	else if (strcmp(tp->name,"cpu_frequency") == 0)
	    tracepoint_probe_unregister( tp, _cpu_frequency, NULL );
	else if (strcmp(tp->name,"cpu_frequency_limits") == 0)
	    tracepoint_probe_unregister( tp, _cpu_frequency_limits, NULL );
	else if (strcmp(tp->name,"cpu_migrate_begin") == 0)
	    tracepoint_probe_unregister( tp, _cpu_migrate_begin, NULL );
	else if (strcmp(tp->name,"mm_migrate_pages") == 0)
	    tracepoint_probe_unregister( tp, _mm_migrate_pages, NULL );
	else if (strcmp(tp->name,"ipi_entry") == 0)
	    tracepoint_probe_unregister( tp, _ipi_entry, NULL );
	else if (strcmp(tp->name,"ipi_exit") == 0)
	    tracepoint_probe_unregister( tp, _ipi_exit, NULL );
	else if (strcmp(tp->name,"timer_expire_entry") == 0)
	    tracepoint_probe_unregister( tp, _timer_expire_entry, NULL );
	else if (strcmp(tp->name,"timer_expire_exit") == 0)
	    tracepoint_probe_unregister( tp, _timer_expire_exit, NULL );
	else if (strcmp(tp->name,"hrtimer_expire_entry") == 0)
	    tracepoint_probe_unregister( tp, _hrtimer_expire_entry, NULL );
	else if (strcmp(tp->name,"hrtimer_expire_exit") == 0)
	    tracepoint_probe_unregister( tp, _hrtimer_expire_exit, NULL );
}
# endif     /* MODULE */
#endif      /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0) */

//static
int  trace_3_sched_switch_hook_add( void )
{
    int err=0;
# if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
	for_each_kernel_tracepoint(regfunc, &err);
# else
    err = register_trace_sched_switch( _sched_switch_hook 
                                      REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sched returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_irq_handler_entry( _hirq_enter REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: hirq_entry returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_irq_handler_exit( _hirq_exit REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: hirq_exit returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_softirq_entry( _sirq_enter REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sirq_entry returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_softirq_exit( _sirq_exit REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sirq_exit returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_sys_enter( _sys_enter REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sys_enter returning %d (0=success)\n", err );
    if (err) return (err);

    err = register_trace_sys_exit( _sys_exit REGISTER_NULL_ARG );
    printk("trace_sched_switch_hook_add: sys_exit returning %d (0=success)\n", err );
# endif
    return (err);
}   // trace_3_sched_switch_hook_add



#ifdef MODULE

static void trace_sched_switch_hook_remove( void )
{
# if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0)
	for_each_kernel_tracepoint(unregfunc, NULL);
# else
    unregister_trace_sys_exit( _sys_exit REGISTER_NULL_ARG );
    unregister_trace_sys_enter( _sys_enter REGISTER_NULL_ARG );
    unregister_trace_softirq_exit( _sirq_exit REGISTER_NULL_ARG );
    unregister_trace_softirq_entry( _sirq_enter REGISTER_NULL_ARG );
    unregister_trace_irq_handler_exit( _hirq_exit REGISTER_NULL_ARG );
    unregister_trace_irq_handler_entry( _hirq_enter REGISTER_NULL_ARG );
    unregister_trace_sched_switch( _sched_switch_hook REGISTER_NULL_ARG );
# endif
}   // trace_sched_switch_hook_remove


static int __init 
#else
int
#endif
trace_3_init(void)
{
    int  ret=0;          /* SUCCESS */
	struct { char tn[TRACE_TN_BUFSZ];	} _trc_;

    //printk("trace_.c:trace_3_init b4 traceInit\n"); // ONLY DO THIS IF module or after console_init()
    if ((ret=traceInit(trace_name(NULL,__FILE__,_trc_.tn,sizeof(_trc_.tn)),0)) != 0) return (ret);
    //printk("trace_.c:trace_3_init after traceInit\n"); // ONLY DO THIS IF module or after console_init()

    //traceTID = 0;

    traceControl_rwp->mode.bits.M = 1;
    TRACE( 64+TLVL_INFO, "kernel trace buffer initialized - no slow path, no timeofday, mode.words.cntl=%u",
	      traceControl_rwp->mode.words.cntl); /* NOTE: don't do slow path (printk) and don't get timeofday (64+) */

    /* 1) create the buffer
       2) create a way to access it (/proc)
       3) register some traces
       4) setup userspace interrupt (sw)
    */
# ifndef MODULE
    return (ret);
# else
    if ((ret=trace_3_proc_add(traceControl_p->memlen))) goto undo1;
    if ((ret=trace_3_sched_switch_hook_add()))          goto undo2;
    return (ret);               /* success */
 undo2:
    trace_proc_remove();
 undo1:
    printk("trace_.c:trace_3_init ERROR before vfree(traceControl_p)\n"); // ONLY DO THIS (printk) IF module
    vfree( traceControl_p );
    return (ret);
# endif
}   // trace_3_init


#ifdef MODULE
static void __exit trace_3_exit(void)
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
}   // trace_3_exit

module_init(trace_3_init);
module_exit(trace_3_exit);

MODULE_AUTHOR("Ron Rechenmacher");
MODULE_DESCRIPTION("Third TRACE");
MODULE_LICENSE("GPL v2"); /* It is for anyone/everyone, I don't care as long as it works for me, and besides I don't want to taint the kernel */
#endif  // MODULE */
