#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "proc.h"

int syscall_num = 0;
int temp_syscall_num = 0;

// /proc/deadlock_syscall_num
ssize_t read_syscall_num(struct file* filp, char* buffer, size_t count, loff_t* offp) {
    char* holder_syscall_num = kmalloc(16 * sizeof(char), GFP_KERNEL);
    if (temp_syscall_num == 0) {
        temp_syscall_num = sprintf(holder_syscall_num, "%d\n", syscall_num);
        copy_to_user(buffer, holder_syscall_num, temp_syscall_num+1);
    } else {
        temp_syscall_num = 0;
    }
    kfree(holder_syscall_num);
    return temp_syscall_num;
}

const struct file_operations proc_deadlock_syscall_num = {
    read : read_syscall_num
};


void init_proc_entries(void) {
    proc_create("deadlock_syscall_num", 0444, NULL, &proc_deadlock_syscall_num);
    printk(KERN_INFO "Loaded proc\n");
}

int proc_init(void) {
    init_proc_entries();
    return 0;
}

void proc_cleanup(void) {
    remove_proc_entry("deadlock_syscall_num", NULL);
}
