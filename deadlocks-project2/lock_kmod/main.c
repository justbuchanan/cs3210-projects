#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "syscall.h"
#include "proc.h"
#include "../custom_syscall.h"

MODULE_LICENSE("GPL");

static struct mutex procs_info_mutex;

struct proc_info {
    pid_t pid;
    pid_t tid;
    unsigned int count;
};

#define MAX_PROCS_SUPPORTED 10
struct proc_info* procs[MAX_PROCS_SUPPORTED];

static struct proc_info* get_proc_info(void) {
    pid_t pid = current->tgid;

    unsigned int freeIndex = -1;

    for(unsigned int i = 0; i < MAX_PROCS_SUPPORTED; i++) {
        if(procs[i]) {
            if(procs[i]->pid == pid) {
                return procs[i];
            }
        }
        else {
            freeIndex = i;
        }
    }

    if(freeIndex == -1) {
        return NULL;
    }

    struct proc_info* proc = kmalloc(sizeof(struct proc_info), GFP_KERNEL);
    proc->pid = pid;
    proc->count = 0;

    procs[freeIndex] = proc;

    return proc;
}

asmlinkage long lock_syscall(int cmd) {
    mutex_lock(&procs_info_mutex);
    struct proc_info* proc = get_proc_info();
    mutex_unlock(&procs_info_mutex);

    if(!proc) {
        printk(KERN_WARNING "COULD NOT CREATE LOCK!\n");
        return -1;
    }

    pid_t tid = current->pid;

    printk(KERN_INFO "lock_syscall(%d) called, cmd = %s, pid = %d, tid = %d\n", cmd, NameForCustomSyscallCommand(cmd), proc->pid, tid);

    switch (cmd) {
        case InitMutex: {
            printk(KERN_INFO "init\n");
            break;
        }
        case LockMutex: {
            mutex_lock(&procs_info_mutex);
            if(proc->tid == tid || proc->count == 0) {
                proc->count++;
                proc->tid = tid;
                mutex_unlock(&procs_info_mutex);
                break;
            }
            else {
                mutex_unlock(&procs_info_mutex);
                return -1;
            }

            printk(KERN_INFO "locked\n");
            break;
        }
        case UnlockMutex: {
            mutex_lock(&procs_info_mutex);
            proc->count--;
            mutex_unlock(&procs_info_mutex);

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

    mutex_init(&procs_info_mutex);
  	
    printk(KERN_INFO "Registered lock_syscall() at %d\n", sysnum);
    return 0;
}

void lock_kmod_cleanup(void) {
    for(unsigned int i = 0; i < MAX_PROCS_SUPPORTED; i++) {
        if(procs[i]) {
            kfree(procs[i]);
        }
    }

    unregister_syscall(sysnum);
    proc_cleanup();
}

module_init(lock_kmod_init);
module_exit(lock_kmod_cleanup);
