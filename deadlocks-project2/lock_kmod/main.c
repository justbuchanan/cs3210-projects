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

int lock_kmod_init(void) {
    init_syscall();
    register_syscall(CustomSyscallNumber, lock_syscall);
    printk(KERN_INFO "Registered lock_syscall() at %d\n", CustomSyscallNumber);
    return 0;
}

void lock_kmod_cleanup(void) {
    unregister_syscall(CustomSyscallNumber);
}

module_init(lock_kmod_init);
module_exit(lock_kmod_cleanup);
