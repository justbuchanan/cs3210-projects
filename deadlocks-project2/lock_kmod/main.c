#include <linux/module.h>
#include <linux/kernel.h>

#include "syscall.h"
#include "../custom_syscall.h"

MODULE_LICENSE("GPL");


asmlinkage long lock_syscall(int cmd) {
    printk("lock_syscall(%d) called\n", cmd);
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
