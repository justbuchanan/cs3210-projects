
#include <unistd.h>
#include <sys/syscall.h>

#include "../../custom_syscall.h"


int main(int argc, char** argv) {
    syscall(CustomSyscallNumber);
    return 0;
}
