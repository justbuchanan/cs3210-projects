#include "monitor.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>

static MonitorEventHandler _handler = NULL;
static int _monitoringUserId = -1;

#define BUF_LEN 1000
static char buffer[BUF_LEN];

void probe_sys_access(const char* filename, int mode) {
    const unsigned long syscall_num = 21;
    int pid = current->pid;
    int tgid = current->tgid;
    int timestamp = -1; // FIXME

    // if (current->uid != _monitoringUserId) return;

    snprintf(buffer, BUF_LEN, "%lu %d %d %d" ", ARGS: %s, %d", syscall_num, pid, tgid, timestamp, filename, mode);
    _handler(buffer);
}
void probe_sys_brk(void) {

}
void probe_sys_chdir(void) {

}
void probe_sys_chmod(void) {

}
void probe_sys_clone(void) {

}
void probe_sys_close(void) {

}
void probe_sys_dup(void) {

}
void probe_sys_dup2(void) {

}
void probe_sys_execve(void) {

}
void probe_sys_exit_group(void) {

}
void probe_sys_fcntl(void) {

}
void probe_sys_fork(void) {

}
void probe_sys_getdents(void) {

}
void probe_sys_getpid(void) {

}
void probe_sys_gettid(void) {

}
void probe_sys_ioctl(void) {

}
void probe_sys_lseek(void) {

}
void probe_sys_mkdir(void) {

}
void probe_sys_mmap(void) {

}
void probe_sys_munmap(void) {

}
void probe_sys_open(void) {

}
void probe_sys_pipe(void) {

}
void probe_sys_read(void) {

}
void probe_sys_rmdir(void) {

}
void probe_sys_select(void) {

}
void probe_sys_stat(void) {

}
void probe_sys_fstat(void) {

}
void probe_sys_lstat(void) {

}
void probe_sys_wait4(void) {

}
void probe_sys_write(void) {

}




// Build a table of all our kprobes, using a macro to make it less verbose
#define PROBE_ENTRY(name) { \
    .entry = probe_sys_##name, \
    .kp = { \
        .symbol_name = "sys_##name" \
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


// // Called just before the probed instruction is executed
// static int _intercept_before(struct kprobe* p, struct pt_regs* regs) {
//     // TODO

//     int ret = 0;
//     kuid_t cuid = current_uid();
//     pid_t pid = task_pid_nr(current);

//     int syscall_num = 1;

//     if (_handler) _handler(syscall_num);

//     return 0;
// }

int num_probes(void) {
    return sizeof(probes) / sizeof(struct kprobe);
}

int monitor_init(MonitorEventHandler handler) {
    int i;

    WARN_ON(handler != NULL);
    _handler = handler;

    for (i = 0; i < num_probes(); ++i) {
        int ret = register_jprobe(&probes[i]);
        if (ret < 0) {
            printk(KERN_INFO "register_jprobe() failed for '%s', returned %d\n", probes[i].kp.symbol_name, ret);
            return -1;
        }
    }

    printk(KERN_INFO "Successfully registered jprobes for syscalls\n");

    return 0;
}

void monitor_cleanup(void) {
    int i;
    for (i = 0; i < num_probes(); ++i) {
        unregister_jprobe(&probes[i]);
    }
    printk(KERN_INFO "Unregistered syscall jprobes\n");
}
