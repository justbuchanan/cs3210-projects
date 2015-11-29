Install the Android NDK

Add the folder to your PATH
export PATH=$PATH:/home/user/Downloads/ndk

Build:
cd android
ndk-build

To Clean:
ndk-build clean

Main Working FUSE APP:
libfuse/mediafs.c

How to run this:
    
cd android/libs/armeabi/
adb push mediafs /sdcard/
adb push fusermount /sdcard/
adb shell
mount -o remount,rw /system
cp /sdcard/mediafs /system/xbin/
chmod 0755 /system/xbin/mediafs
cp /sdcard/fusermount /system/xbin/
chmod 0755 /system/xbin/fusermount

mkdir /system/fuse
cd system/xbin
./mediafs /system/fuse -o allow_other

Now you can use your adb shell to go into the folder and ls
