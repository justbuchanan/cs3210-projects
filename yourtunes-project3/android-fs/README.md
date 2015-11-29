Install the Android NDK

Add the folder to your PATH
export PATH=$PATH:/home/user/Downloads/ndk

Build:
cd android
ndk-build

To Clean:
ndk-build clean

Files to edit:
libfuse/fusexmp.c


How to run this:
    
cd android/libs/armeabi/
adb push fusexmp /sdcard/
adb push fusermount /sdcard/
adb shell
mount -o remount,rw /system
cp /sdcard/fusexmp /system/xbin/
chmod 0755 /system/xbin/fusexmp
cp /sdcard/fusermount /system/xbin/
chmod 0755 /system/xbin/fusermount

mkdir /system/fuse
cd system/xbin
./fusexmp /system/fuse

Now you can use your adb shell to go into the folder and ls
