#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/sched.h>

#include "syscall.h"
#include "proc.h"
#include "../custom_syscall.h"

MODULE_LICENSE("GPL");

struct proc_mutex_info {
    pid_t pid;
    struct mutex bestoffer_mutex;
    unsigned int count;
};

#define MAX_PROCS_SUPPORTED 10
struct proc_mutex_info* procs[MAX_PROCS_SUPPORTED];

asmlinkage long lock_syscall(int cmd) {
    pid_t pid = task_pid_nr(current);

    printk(KERN_INFO "lock_syscall(%d) called, cmd = %s, pid = %d\n", cmd, NameForCustomSyscallCommand(cmd), pid);

    switch (cmd) {
        case InitMutex: {
           // mutex_init(&bestoffer_mutex);
            printk(KERN_INFO "init\n");
            break;
        }
        case LockMutex: {
            //mutex_lock(&bestoffer_mutex);
            printk(KERN_INFO "locked\n");
            break;
        }
        case UnlockMutex: {
            //mutex_unlock(&bestoffer_mutex);
            printk(KERN_INFO "unlocked \n");
            break;
        }
        default: {
            printk(KERN_INFO "Ignoring command\n");
        }
    }

    return 0;
}

// Global variable for the table index where our syscall is registered
static int sysnum = -1;

int lock_kmod_init(void) {
    init_syscall();
    proc_init();
    sysnum = register_syscall(lock_syscall);
    if (sysnum < 0) {
        printk(KERN_WARNING "Unable to register syscall\n");
        return -1;
    }
  	syscall_num = sysnum;
  	
    printk(KERN_INFO "Registered lock_syscall() at %d\n", sysnum);
    return 0;
}

void lock_kmod_cleanup(void) {
    unregister_syscall(sysnum);
    proc_cleanup();
}

module_init(lock_kmod_init);
module_exit(lock_kmod_cleanup);
