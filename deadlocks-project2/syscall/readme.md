
# Syscall

This module registers a system call at a pre-determined location in the kernel syscall table.

## Testing

To test the module, run `make load` in this directory to build the module and load it into the kernel.  It will automatically register an example syscall.

To try it out, `cd` into the `test` directory and run `make run`.  This will build a small executable that calls the recently-registered sycall.  You can check to see that it worked by looking at the output from `dmesg`.