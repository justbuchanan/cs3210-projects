# sysmon-3210
Linux kernel module for logging syscalls

## Usage

There is a makefile in this repo for building, loading and unloading the sysmon kernel module.

~~~{.sh}
# running `make` with no arguments builds the module
make

# Build and load the module into the kernel
make load

# Unload the module from the kernel
make unload
~~~

## Testing

There is a test script in the repo named [test.sh](test.sh).  It accepts a command to run as its argument and will print out the syscall log resulting from running the command.  It does this by creating a test user account and setting sysmon to only log calls from that user.  It then executes the provided command as that user and returns the logged results.
