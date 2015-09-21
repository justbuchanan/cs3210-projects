#include "monitor.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

#include <stdarg.h>

static MonitorEventHandler _handler = NULL;
static int _monitoring_user_id = -1;

void monitor_set_uid(int uid) { _monitoring_user_id = uid; }

int monitor_get_uid(void) { return _monitoring_user_id; }

#define BUF_LEN 1000
static char buffer[BUF_LEN] = {'\0'};

// Sends a logline through the global callback function consisting including the
// additional parameters in the format specified.
void send_logline(unsigned long syscallNum, const char* fmt, ...) {
    int uid = get_current_user()->uid.val;
    if (uid != _monitoring_user_id) return;

    if (!_handler) return;

    int pid = current->pid;
    int tgid = current->tgid;

    // TODO: timestamp
    // TODO: mutex

    // print syscall num, pid, tgid
    int num_written =
        snprintf(buffer, BUF_LEN, "%lu %d %d, ARGS: ", syscallNum, pid, tgid);

    // Print the arguments to the syscall
    if (fmt) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer + num_written, BUF_LEN - num_written, fmt, args);
    }

    _handler(buffer);
}

// Each jprobe has a separate callback function with the same signature as the
// syscall itself.
int probe_sys_access(const char* filename, int mode) {
    send_logline(__NR_access, "%s, %d", filename, mode);
    jprobe_return();
    return 0;
}
int probe_sys_brk(unsigned long addr) {
    send_logline(__NR_brk, "%lu", addr);
    jprobe_return();
    return 0;
}
int probe_sys_chdir(const char* path) {
    send_logline(__NR_chdir, "%s", path);
    jprobe_return();
    return 0;
}
int probe_sys_chmod(const char* filename, mode_t mode) {
    send_logline(__NR_chmod, "%s, %lu", filename, mode);  // TODO: mode_t?
    jprobe_return();
    return 0;
}
int probe_sys_clone(void) {  // TODO: fix
    send_logline(__NR_clone, "");
    jprobe_return();
    return 0;
}
int probe_sys_close(unsigned int fd) {
    send_logline(__NR_close, "%lu", fd);
    jprobe_return();
    return 0;
}
int probe_sys_dup(unsigned int fildes) {
    send_logline(__NR_dup, "%u", fildes);
    jprobe_return();
    return 0;
}
int probe_sys_dup2(unsigned int oldfd, unsigned int newfd) {
    send_logline(__NR_dup2, "%u, %u", oldfd, newfd);
    jprobe_return();
    return 0;
}
int probe_sys_execve(char* whatisthis, char** hsdfasd, char** asdfas,
                     struct pt_regs* sdfafs) {
    send_logline(__NR_execve, "");  // TODO
    jprobe_return();
    return 0;
}
int probe_sys_exit_group(int error_code) {
    send_logline(__NR_exit_group, "%d", error_code);
    jprobe_return();
    return 0;
}
int probe_sys_fcntl(unsigned int fd, unsigned int cmd, unsigned long arg) {
    send_logline(__NR_fcntl, "%u, %u, %lu", fd, cmd, arg);
    jprobe_return();
    return 0;
}
int probe_sys_fork(struct pt_regs* regs) {
    send_logline(__NR_fork, "");  // TODO
    jprobe_return();
    return 0;
}
int probe_sys_getdents(unsigned int fd, struct linux_dirent* dirent,
                       unsigned int count) {
    send_logline(__NR_getdents, "%u, %x, %p", fd, dirent, count);
    jprobe_return();
    return 0;
}
int probe_sys_getpid(void) {
    send_logline(__NR_getpid, "");
    jprobe_return();
    return 0;
}
int probe_sys_gettid(void) {
    send_logline(__NR_gettid, "");
    jprobe_return();
    return 0;
}
int probe_sys_ioctl(unsigned int fd, unsigned int cmd, unsigned long arg) {
    send_logline(__NR_ioctl, "%u, %u, %lu", fd, cmd, arg);
    jprobe_return();
    return 0;
}
int probe_sys_lseek(unsigned int fd, off_t offset, unsigned int origin) {
    send_logline(__NR_lseek, "%u, %?, %u", fd, offset, origin);  // TODO
    jprobe_return();
    return 0;
}
int probe_sys_mkdir(const char* pathname, int mode) {
    send_logline(__NR_mkdir, "%s, %d", pathname, mode);
    jprobe_return();
    return 0;
}
int probe_sys_mmap(void) {  // TODO
    send_logline(__NR_mmap, "");
    jprobe_return();
    return 0;
}
int probe_sys_munmap(unsigned long addr, size_t len) {  // TODO
    send_logline(__NR_munmap, "%lu, %d", addr, len);
    jprobe_return();
    return 0;
}
int probe_sys_open(const char* filename, int flags, int mode) {
    send_logline(__NR_open, "%s, %d, %d", filename, flags, mode);
    jprobe_return();
    return 0;
}
int probe_sys_pipe(int* fildes) {
    send_logline(__NR_pipe, "%p", fildes);
    jprobe_return();
    return 0;
}
int probe_sys_read(unsigned int fd, char* buf, size_t count) {
    send_logline(__NR_read, "%u, %s, %d", fd, buf, count);
    jprobe_return();
    return 0;
}
int probe_sys_rmdir(const char* pathname) {
    send_logline(__NR_rmdir, "%s", pathname);
    jprobe_return();
    return 0;
}
int probe_sys_select(int n, fd_set* inp, fd_set* outp, fd_set* exp,
                     struct timeval* tvp) {
    send_logline(__NR_select, "");  // TODO
    jprobe_return();
    return 0;
}
int probe_sys_stat(struct __old_kernel_stat* statbuf) {  // TODO
    send_logline(__NR_stat, "%p", statbuf);
    jprobe_return();
    return 0;
}
int probe_sys_fstat(unsigned int fd, struct __old_kernel_stat* statbuf) {
    send_logline(__NR_fstat, "%u, %p", fd, statbuf);
    jprobe_return();
    return 0;
}
int probe_sys_lstat(char* filename, struct __old_kernel_stat* statbuf) {
    send_logline(__NR_lstat, "%s, %p", filename, statbuf);
    jprobe_return();
    return 0;
}
int probe_sys_wait4(pid_t pid, int* stat_addr, int options, struct rusage* ru) {
    send_logline(__NR_wait4, "%d, %p, %d, %p", pid, stat_addr, options, ru);
    jprobe_return();
    return 0;
}
int probe_sys_write(unsigned int fd, const char* buf, size_t count) {
    send_logline(__NR_write, "%u, %s, %d", fd, buf, count);
    jprobe_return();
    return 0;
}

// Build a table of all our kprobes, using a macro to make it less verbose
#define PROBE_ENTRY(name)                                              \
    {                                                                  \
        .entry = probe_sys_##name, .kp = {.symbol_name = "sys_" #name} \
    }
struct jprobe probes[] = {
    PROBE_ENTRY(access),  // char*, int
    PROBE_ENTRY(brk),     // long
    PROBE_ENTRY(chdir),   // char*
    PROBE_ENTRY(chmod),   // const char*, umode_t
    PROBE_ENTRY(clone),   //
    PROBE_ENTRY(close),  PROBE_ENTRY(dup),        PROBE_ENTRY(dup2),
    PROBE_ENTRY(execve), PROBE_ENTRY(exit_group), PROBE_ENTRY(fcntl),
    PROBE_ENTRY(fork),   PROBE_ENTRY(getdents),   PROBE_ENTRY(getpid),
    PROBE_ENTRY(gettid), PROBE_ENTRY(ioctl),      PROBE_ENTRY(lseek),
    PROBE_ENTRY(mkdir),  PROBE_ENTRY(mmap),       PROBE_ENTRY(munmap),
    PROBE_ENTRY(open),   PROBE_ENTRY(pipe),       PROBE_ENTRY(read),
    PROBE_ENTRY(rmdir),  PROBE_ENTRY(select),     PROBE_ENTRY(stat),
    PROBE_ENTRY(fstat),  PROBE_ENTRY(lstat),      PROBE_ENTRY(wait4),
    PROBE_ENTRY(write)};

#define NUM_PROBES (sizeof(probes) / sizeof(struct jprobe))

int monitor_init(MonitorEventHandler handler) {
    if (handler == NULL) {
        printk(KERN_WARNING "Invalid handler given to monitor_init()");
        return -1;
    }
    _handler = handler;

    for (int i = 0; i < NUM_PROBES; ++i) {
        int ret = register_jprobe(&probes[i]);
        if (ret < 0) {
            printk(KERN_INFO "register_jprobe() failed for '%s', returned %d\n",
                   probes[i].kp.symbol_name, ret);
            return -1;
        }
    }

    printk(KERN_INFO "Successfully registered jprobes for syscalls\n");

    return 0;
}

void monitor_cleanup(void) {
    for (int i = 0; i < NUM_PROBES; ++i) {
        unregister_jprobe(&probes[i]);
    }
    printk(KERN_INFO "Unregistered syscall jprobes\n");
}
