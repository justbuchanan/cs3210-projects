#include "monitor.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/unistd.h>

#include <stdarg.h>

static MonitorEventHandler _handler = NULL;
static unsigned short _monitoring_user_id = -1;

#define BUF_LEN 1000
static char buffer[BUF_LEN] = {'\0'};

void send_logline(unsigned long syscallNum, const char* fmt, ...) {
    unsigned int uid = get_current_user()->uid.val;
    if (uid != _monitoring_user_id) return;

    int pid = current->pid;
    int tgid = current->tgid;

    // print syscall num, pid, tgid
    int num_written = snprintf(buffer, BUF_LEN, "%lu %d %d ", syscallNum, pid, tgid);

    // Print the arguments to the syscall
    if (fmt) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer+num_written, BUF_LEN-num_written, fmt, args);
    }

    _handler(buffer);
}

int probe_sys_access(const char* filename, int mode) {
    send_logline(__NR_access, "ARGS: %s, %d", filename, mode);
    jprobe_return();
    return 0;
}
int probe_sys_brk(void) {

}
int probe_sys_chdir(void) {

}
int probe_sys_chmod(void) {

}
int probe_sys_clone(void) {

}
int probe_sys_close(void) {

}
int probe_sys_dup(void) {

}
int probe_sys_dup2(void) {

}
int probe_sys_execve(void) {

}
int probe_sys_exit_group(void) {

}
int probe_sys_fcntl(void) {

}
int probe_sys_fork(void) {

}
int probe_sys_getdents(void) {

}
int probe_sys_getpid(void) {

}
int probe_sys_gettid(void) {

}
int probe_sys_ioctl(void) {

}
int probe_sys_lseek(void) {

}
int probe_sys_mkdir(void) {

}
int probe_sys_mmap(void) {

}
int probe_sys_munmap(void) {

}
int probe_sys_open(void) {

}
int probe_sys_pipe(void) {

}
int probe_sys_read(void) {

}
int probe_sys_rmdir(void) {

}
int probe_sys_select(void) {

}
int probe_sys_stat(void) {

}
int probe_sys_fstat(void) {

}
int probe_sys_lstat(void) {

}
int probe_sys_wait4(void) {

}
int probe_sys_write(void) {

}




// Build a table of all our kprobes, using a macro to make it less verbose
#define PROBE_ENTRY(name) { \
    .entry = probe_sys_##name, \
    .kp = { \
        .symbol_name = "sys_"#name \
    } \
}
struct jprobe probes[] = {
    PROBE_ENTRY(access), // char*, int
    PROBE_ENTRY(brk),    // long
    PROBE_ENTRY(chdir),  // char*
    PROBE_ENTRY(chmod),  // const char*, umode_t
    PROBE_ENTRY(clone),  //
    PROBE_ENTRY(close),
    PROBE_ENTRY(dup),
    PROBE_ENTRY(dup2),
    PROBE_ENTRY(execve),
    PROBE_ENTRY(exit_group),
    PROBE_ENTRY(fcntl),
    PROBE_ENTRY(fork),
    PROBE_ENTRY(getdents),
    PROBE_ENTRY(getpid),
    PROBE_ENTRY(gettid),
    PROBE_ENTRY(ioctl),
    PROBE_ENTRY(lseek),
    PROBE_ENTRY(mkdir),
    PROBE_ENTRY(mmap),
    PROBE_ENTRY(munmap),
    PROBE_ENTRY(open),
    PROBE_ENTRY(pipe),
    PROBE_ENTRY(read),
    PROBE_ENTRY(rmdir),
    PROBE_ENTRY(select),
    PROBE_ENTRY(stat),
    PROBE_ENTRY(fstat),
    PROBE_ENTRY(lstat),
    PROBE_ENTRY(wait4),
    PROBE_ENTRY(write)
};

int num_probes(void) {
    return sizeof(probes) / sizeof(struct jprobe);
}

static struct jprobe access_probe = {
    .entry = probe_sys_access,
    .kp = {
        .symbol_name = "sys_access"
    }
};

int monitor_init(MonitorEventHandler handler) {
    if (handler == NULL) {
        // WARN_ON(handler == NULL);
        printk(KERN_WARNING "Invalid handler given to monitor_init()");
        return -1;
    }

    _handler = handler;

    int ret = register_jprobe(&access_probe);
    if (ret < 0) {
        // printk(KERN_INFO "register_jprobe() failed for '%s', returned %d\n", probes[i].kp.symbol_name, ret);
        printk(KERN_WARNING "register_jprobe() failed\n");
        return -1;
    }

    // for (int i = 0; i < num_probes(); ++i) {
    //     int ret = register_jprobe(&probes[i]);
    //     if (ret < 0) {
    //         printk(KERN_INFO "register_jprobe() failed for '%s', returned %d\n", probes[i].kp.symbol_name, ret);
    //         return -1;
    //     }
    // }

    printk(KERN_INFO "Successfully registered jprobes for syscalls\n");

    return 0;
}

void monitor_cleanup(void) {
    // int i;
    // for (i = 0; i < num_probes(); ++i) {
    //     unregister_jprobe(&probes[i]);
    // }
    unregister_jprobe(&access_probe);
    printk(KERN_INFO "Unregistered syscall jprobes\n");
}
