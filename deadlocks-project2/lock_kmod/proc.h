#ifndef PROC_H
#define PROC_H

static int syscall_num;

// Initialize proc file
int proc_init(void);

// Cleanup resources used by proc file
void proc_cleanup(void);

#endif // PROC_H
