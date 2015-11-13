#ifndef CUSTOM_SYSCALL_H
#define CUSTOM_SYSCALL_H


typedef enum {
    AllocateMutex = 1,
    InitMutex,
    DestroyMutex,
    LockMutex,
    UnlockMutex,
} CustomSyscallCommand;


const char* NameForCustomSyscallCommand(CustomSyscallCommand cmd) {
    const char* names[] = {
        "Null",
        "AllocateMutex",
        "InitMutex",
        "DestroyMutex",
        "LockMutex",
        "UnlockMutex"
    };
    return names[cmd];
}


#endif // CUSTOM_SYSCALL_H
