#ifndef CUSTOM_SYSCALL_H
#define CUSTOM_SYSCALL_H

// The index in the table we're registering our syscall at
// note: the current value (321) probably isn't good - we need to find an empty slot
const int CustomSyscallNumber = 321;


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
