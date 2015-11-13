#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/syscalls.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <asm/pgtable.h>
#include <asm/asm-offsets.h>

/** Dynamically Adding a Syscall **/
const char RET_OPOCDE = '\xc3';

static pte_t *pte;
static unsigned long **sys_call_table;
static unsigned long *sys_ni_syscall_ptr;

static int ready_to_work;

static size_t no_syscall_len;

/* A local copy of sys_ni_syscall */
static asmlinkage long no_syscall(void){
	return -ENOSYS;
}

static inline void set_no_syscall_len(void){
	int i = 0;
	
	/* figure out the size of function */
	while(((char *)no_syscall)[i] != RET_OPOCDE)
		i++;
	
	no_syscall_len = (i + 1);
}

/* Restore kernel memory page protection */
static inline void protect_memory(void){
	set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
}

/* Unprotected kernel memory page containing for writing */
static inline void unprotect_memory(void){
	set_pte_atomic(pte, pte_mkwrite(*pte));
}

/* search by __NR_close in all kernel memory */
static inline unsigned long **find_syscall_table(void){
	unsigned long **sys_table;
	unsigned long offset = PAGE_OFFSET;
	
	while(offset < ULONG_MAX){
		sys_table = (unsigned long **)offset;
		
		if(sys_table[__NR_close] == (unsigned long *)sys_close)
			return sys_table;
		
		offset += sizeof(void *);
	}
	
	return NULL;
}

static inline int find_free_position(void){
	int i;
	int pos = -1;
	
	for(i = 0; i <= __NR_syscall_max; i++){
		if(memcmp(sys_call_table[i], no_syscall, no_syscall_len) == 0){
			pos = i;
			break;
		}
	}
	
	return pos;
}

int register_syscall(void *fptr){
	int sysnum;
	spinlock_t my_lock;
	
	spin_lock_init(&my_lock);
	
	/* sanity checks */
	if((!ready_to_work) || (!fptr))
		return -1;
	
	//lock
	spin_lock(&my_lock);
	
	if((sysnum = find_free_position()) < 0)
		return -1;
	
	unprotect_memory();
	sys_call_table[sysnum] = fptr;
	protect_memory();
	
	//unlock
	spin_unlock(&my_lock);
	
	return sysnum;
}

void unregister_syscall(int sysnum){
	/* sanity check */
	if(!ready_to_work) return;
	
	unprotect_memory();
	sys_call_table[sysnum] = sys_ni_syscall_ptr;
	protect_memory();
}

int init_syscall(void){
	unsigned int level;
	int i = 0;
	
	sys_ni_syscall_ptr = NULL;
	ready_to_work = 0;
	
	sys_call_table = find_syscall_table();
	if(!sys_call_table){
		printk(KERN_INFO "sys_call_table[] was not found\n");
		return -1;
	}
	
	if(!(pte = lookup_address((unsigned long)sys_call_table, &level)))
		return -1;
	
	set_no_syscall_len();
	
	if((i = find_free_position()) < 0)
		return -1;
	
	/* retrieves the original pointer to sys_ni_syscall */
	sys_ni_syscall_ptr = sys_call_table[i];
	
	/* ok lets work */
	ready_to_work = 1;
	
	return 0;
}

// /* Custom SysCall function */
// asmlinkage long syscall_hello(int i, char* str) {
//     printk(KERN_INFO "This message is brought to you by a dynamic syscall\n");
//     printk(KERN_INFO "Param 1 is: %d, Param 2 is: %s\n", i, str);
//     return 0;
// }


// int syscall_init(void) {
//     init_syscall();
//     int sysnum;

// 	sysnum = register_syscall(syscall_hello);
// 	if(sysnum < 0){
// 		printk(KERN_INFO "[syscall_hello] was not registered\n");
// 		return -1;
// 	}
//     // TODO - use sysnum and store it somewhere
// 	printk(KERN_INFO "[syscall_hello] registered in [%d]\n", sysnum);
// 	return 0;
// }
