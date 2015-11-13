#ifndef SYSCALL_H
#define SYSCALL_H

// this must be called before registering or unregistering syscalls
int init_syscall(void);

// registers a syscall for the given function at the given location in the table
int register_syscall(void *fptr);

void unregister_syscall(int sysnum);


#endif // SYSCALL_H
