
#include <unistd.h>
#include <sys/syscall.h>

int main(int argc, char** argv) {
    syscall(321);
    return 0;
}
