#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#include "monitor.h"

MODULE_LICENSE("GPL");

int toggle = 0;

int temp_uid;
int temp_toggle;


void monitor_handler(const char*);


// /proc/sysmon_uid
ssize_t read_uid(struct file* filp, char* buffer, size_t count, loff_t* offp) {
    char* holder_uid = kmalloc(GFP_KERNEL, 16 * sizeof(char));
    if (temp_uid == 0) {
	temp_uid = sprintf(holder_uid, "%d\n", monitor_get_uid());
        copy_to_user(buffer, holder_uid, temp_uid);
    } else {
	temp_uid = 0;
    }
    kfree(holder_uid);
    return temp_uid;
}
ssize_t write_uid(struct file* filp, const char* buffer, size_t count,
                  loff_t* offp) {
    long val;

    char* holder_uid = kmalloc(GFP_KERNEL, 16 * sizeof(char));

    if (count == 0 || count == 1 || count > 15){
	printk(KERN_ERR "Invalid UID value!\n");
	return -EINVAL;
    }

    copy_from_user(holder_uid, buffer, count);
    holder_uid[count] = '\0';

    int success = kstrtol(holder_uid, 10, &val);

    kfree(holder_uid);

    if (success == 0 && val > 0) {
        monitor_set_uid(val);
    } else {
	printk(KERN_ERR "Invalid UID value!\n");
        return -EINVAL;
    }

    return count;
}

const struct file_operations proc_sysmon_uid = {
    read : read_uid,
    write : write_uid
};

// /proc/sysmon_toggle
ssize_t read_toggle(struct file* filp, char* buffer, size_t count,
                    loff_t* offp) {
    char* holder_toggle = kmalloc(GFP_KERNEL, 16 * sizeof(char));
    if (temp_toggle == 0) {
	temp_toggle = sprintf(holder_toggle, "%d\n", toggle);
        copy_to_user(buffer, holder_toggle, temp_toggle);
    } else {
	temp_toggle = 0;
    }
    kfree(holder_toggle);
    return temp_toggle;
}
void set_toggle(long new_toggle){
    if (toggle != new_toggle){
	if (new_toggle == 0)
	    monitor_cleanup();
	else 
	    monitor_init(&monitor_handler);
	toggle = new_toggle;
    }
}
ssize_t write_toggle(struct file* filp, const char* buffer, size_t count,
                     loff_t* offp) {
    long val;

    char* holder_toggle = kmalloc(GFP_KERNEL, 2 * sizeof(char));

    if (count != 2) {
        printk(KERN_ERR "Toggle value must be one bit (1 or 0)!\n");
	return -EINVAL;
    }

    copy_from_user(holder_toggle, buffer, 1);
    holder_toggle[1] = '\0';
    int success = kstrtol(holder_toggle, 10, &val);

    kfree(holder_toggle);

    if (success == 0 && (val == 0 || val == 1)) {
	set_toggle(val);
    } else {
        printk(KERN_ERR "Toggle value must be one bit (1 or 0)!\n");
        return -EINVAL;
    }

    return count;
}
const struct file_operations proc_sysmon_toggle = {
    read : read_toggle,
    write : write_toggle
};

// /proc/log
ssize_t read_log(struct file* filp, char* buffer, size_t count,
                    loff_t* offp) {
    // TODO
    return 0;
}
const struct file_operations proc_sysmon_log = {
    read : read_log
};

void init_proc_entries(void) {
    proc_create("sysmon_uid", 0600, NULL, &proc_sysmon_uid);
    proc_create("sysmon_toggle", 0600, NULL, &proc_sysmon_toggle);
    proc_create("sysmon_log", 0400, NULL, &proc_sysmon_log);
    printk(KERN_INFO "Loaded procs");
}

void monitor_handler(const char* logline) {
    printk(KERN_INFO "Monitor: %s\n", logline);
}

int proc_init(void) {
    init_proc_entries();
    if (toggle == 1)
        monitor_init(&monitor_handler);
    return 0;
}

void proc_cleanup(void) {
    monitor_cleanup();
    remove_proc_entry("sysmon_uid", NULL);
    remove_proc_entry("sysmon_toggle", NULL);
    remove_proc_entry("sysmon_log", NULL);
}

module_init(proc_init);
module_exit(proc_cleanup);
