#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>

#include "syscall.h"
#include "../custom_syscall.h"

MODULE_LICENSE("GPL");



struct mutex bestoffer_mutex;


asmlinkage long lock_syscall(int cmd) {
    printk("lock_syscall(%d) called, cmd = %s\n", cmd, NameForCustomSyscallCommand(cmd));

    switch (cmd) {
        case InitMutex: {
            mutex_init(&bestoffer_mutex);
            printk(KERN_INFO "init\n");
            break;
        }
        case LockMutex: {
            mutex_lock(&bestoffer_mutex);
            printk(KERN_INFO "locked\n");
            break;
        }
        case UnlockMutex: {
            mutex_unlock(&bestoffer_mutex);
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
    sysnum = register_syscall(lock_syscall);
    if (sysnum < 0) {
        printk(KERN_WARNING "Unable to register syscall\n");
        return -1;
    }

    printk(KERN_INFO "Registered lock_syscall() at %d\n", sysnum);
    return 0;
}

void lock_kmod_cleanup(void) {
    unregister_syscall(sysnum);
}

module_init(lock_kmod_init);
module_exit(lock_kmod_cleanup);
