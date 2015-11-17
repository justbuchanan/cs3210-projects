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

// A local copy of sys_ni_syscall
// This is basically a syscall that does nothing (hence available)
static asmlinkage long no_syscall(void){
	return -ENOSYS;
}

// Counts the size of the function by going through the function
// byte by byte until the RETURN opcode is found
static inline void set_no_syscall_len(void){
	int i = 0;
	while(((char *)no_syscall)[i] != RET_OPOCDE)
		i++;
	no_syscall_len = (i + 1);
}

// Protects the kernel memory page by clearing the RW flag
static inline void protect_memory(void){
	set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
}

// Unprotect the kernel memory page so that it can be written to (modifying the sycall table)
static inline void unprotect_memory(void){
	set_pte_atomic(pte, pte_mkwrite(*pte));
}

// Find the syscall table by finding the memory location at
// which the sys_close function is defined in the table.
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

// Finds the position of a ni_syscall function
// The ni_syscall function is just a function that is not implemented
// and we use this info for adding our syscall function in its place.
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

// This registers the syscall by adding our syscall function in the place
// of the ni_syscall function in the syscall table.
int register_syscall(void *fptr){
	int sysnum;
	spinlock_t my_lock;
	
	spin_lock_init(&my_lock);
	
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

// Removes the dynamic syscall by resetting the function
// pointer to the ni_syscall function.
void unregister_syscall(int sysnum){
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
	
	sys_ni_syscall_ptr = sys_call_table[i];
	
    // Init complete
	ready_to_work = 1;
	
	return 0;
}
