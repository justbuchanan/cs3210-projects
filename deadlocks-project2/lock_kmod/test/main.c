
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>

#include "../../custom_syscall.h"


int main(int argc, char** argv) {
    if (argc == 1) {
        fprintf(stderr, "Error, no syscall number provided\n");
        return -1;
    }
	
    printf("PID is %d\n", getpid());
    printf("TID is %lu\n", syscall(SYS_gettid));

    int sysnum = atoi(argv[1]);
    int param = 1;
    if (argc > 2) {
        param = atoi(argv[2]);
    }
    syscall(sysnum, param);
    return 0;
}
