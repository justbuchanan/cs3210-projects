
# Deadlock Management System

This project contains two binaries: a kernel module that handles locking/unlocking mutexes and a userspace library that interfaces with it.  The code for the kernel module is in the `lock_kmod` directory and the library consists of the `class_thread.{h, c}` files.

To try the project, do:

~~~
make run
~~~

This will compile the kernel module and load it into the kernel.  It also builds the userspace library and runs the `bestoffer` binary after linking.  You should see the program run indefinitely because the locking issues have been resolved.
