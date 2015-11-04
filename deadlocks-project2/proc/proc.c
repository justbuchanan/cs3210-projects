#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

pid_t pid = 0;
int temp_pid = 0;

// /proc/deadlock_pid
ssize_t read_pid(struct file* filp, char* buffer, size_t count, loff_t* offp) {
    char* holder_pid = kmalloc(GFP_KERNEL, 16 * sizeof(char));
    if (temp_pid == 0) {
        temp_pid = sprintf(holder_pid, "%d\n", pid);
        copy_to_user(buffer, holder_pid, temp_pid);
    } else {
        temp_pid = 0;
    }
    kfree(holder_pid);
    return temp_pid;
}

ssize_t write_pid(struct file* filp, const char* buffer, size_t count,
                  loff_t* offp) {
    long val;

    char* holder_pid = kmalloc(16 * sizeof(char), GFP_KERNEL);

    if (count == 0 || count == 1 || count > 15) {
        printk(KERN_ERR "Invalid PID value!\n");
        return -EINVAL;
    }

    copy_from_user(holder_pid, buffer, count);
    holder_pid[count] = '\0';

    int success = kstrtol(holder_pid, 10, &val);

    kfree(holder_pid);

    if (success == 0 && val >= 0) {
	pid = val;
    } else {
        printk(KERN_ERR "Invalid UID value!\n");
        return -EINVAL;
    }

    return count;
}

const struct file_operations proc_deadlock_pid = {
    read : read_pid,
    write : write_pid
};


void init_proc_entries(void) {
    proc_create("deadlock_pid", 0600, NULL, &proc_deadlock_pid);
    printk(KERN_INFO "Loaded procs\n");
}

int proc_init(void) {
    init_proc_entries();
    return 0;
}

void proc_cleanup(void) {
    remove_proc_entry("deadlock_pid", NULL);
}

module_init(proc_init);
module_exit(proc_cleanup);
