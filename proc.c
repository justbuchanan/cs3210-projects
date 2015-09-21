#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include "monitor.h"

MODULE_LICENSE("GPL");

int toggle = 0;

char* holder_uid;
char* holder_toggle;

int len_uid, temp_uid;
int len_toggle, temp_toggle;

// /proc/sysmon_uid
ssize_t read_uid(struct file* filp, char* buffer, size_t count, loff_t* offp) {
    char str[5];
    if (temp_uid > 0) {
        temp_uid = 0;
        return 0;
    }
    temp_uid = sprintf(str, "%d\n", monitor_get_uid());
    copy_to_user(buffer, str, temp_uid);
    return temp_uid;
}
ssize_t write_uid(struct file* filp, const char* buffer, size_t count,
              loff_t* offp) {
    long val;

    copy_from_user(holder_uid, buffer, count);
    holder_uid[count] = '\0';
    if (kstrtol(holder_uid, 10, &val) == 0) {
        if (val <= 0) return -EINVAL;
        monitor_set_uid(val);
        len_uid = count;
    } else
        return -EINVAL;
    return count;
}

const struct file_operations proc_sysmon_uid = {read : read_uid, write : write_uid};

// sysmon_toggle
ssize_t read_toggle(struct file* filp, char* buffer, size_t count, loff_t* offp) {
    char str[5];
    if (temp_toggle > 0) {
        temp_toggle = 0;
        return 0;
    }
    temp_toggle = sprintf(str, "%d\n", toggle);
    copy_to_user(buffer, str, temp_toggle);
    return temp_toggle;
}
ssize_t write_toggle(struct file* filp, const char* buffer, size_t count,
                 loff_t* offp) {
    long val;

    copy_from_user(holder_toggle, buffer, count);
    holder_toggle[count] = '\0';
    if (kstrtol(holder_toggle, 10, &val) == 0) {
        if (val != 0 || val != 1) return -EINVAL;
        toggle = val;
        len_toggle = count;
    } else
        return -EINVAL;
    return count;
}

const struct file_operations proc_sysmon_toggle = {
    read : read_toggle,
    write : write_toggle
};

void init_proc_entries(void) {
    proc_create("sysmon_uid", 0600, NULL, &proc_sysmon_uid);
    proc_create("sysmon_toggle", 0600, NULL, &proc_sysmon_toggle);
    holder_uid = kmalloc(GFP_KERNEL, 16 * sizeof(char));
    holder_toggle = kmalloc(GFP_KERNEL, 16 * sizeof(char));
    printk(KERN_INFO "Loaded procs");
}

void monitor_handler(const char* logline) {
    printk(KERN_INFO "Monitor: %s\n", logline);
}

int proc_init(void) {
    init_proc_entries();
    monitor_init(&monitor_handler);
    return 0;
}

void proc_cleanup(void) {
    monitor_cleanup();
    remove_proc_entry("sysmon_uid", NULL);
    remove_proc_entry("sysmon_toggle", NULL);
}

module_init(proc_init);
module_exit(proc_cleanup);
