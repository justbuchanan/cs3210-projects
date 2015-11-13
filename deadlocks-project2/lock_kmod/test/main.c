
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>

#include "../../custom_syscall.h"


int main(int argc, char** argv) {
    int param = 1;
    if (argc > 1) {
        param = atoi(argv[1]);
    }
    syscall(CustomSyscallNumber, param);
    return 0;
}
