# Android hal driver for gps and bds
An android hal driver, support for both gps and bds satellites system.

Build and tested on android4.x.

Files:

/hardware/libgps/gps_zkw_v3.c
/hardware/libgps/android.mk
/frameworks/base/location/java/android/locaton/GpsStatus.java

As android's original location service only support prn from 1 to 32(for it's use an int32 to hold all the satellites's in_use_fix_flag), that's only enough for gps system. 

To support bds system, we need something more tricky.

As satellite's azimuth is always in range [0, 360), but with a float type, that's quite enough to hold something more.

So I add in_use_fix_flag to azimuth.

If the satellite is used in fix, it's azimuth will plus 720, else nothing changed.

And when the LocationAPI want the satellite's status, if it's azimuth if bigger than 720, then it's used in fix, else it's not. 

As we do all the things in android's hal(parse nmea and conceal satellites's in_use_fix_flag) and framework(reveal satellites's in_use_fix_flag and restore azimuth to normal), so any 3rd party application can work with it.

#Requirements
1. Android source code
2. Android build enviroment.
3. You must first build the full android source.

*How to build android source is not a topic here.

#How to use
##Sepcial note
This driver in under origin android system, not suite for MTK version, and other 
##HAL level

1. Copy folder /hardware/libgps to android source path
2. Edit gps_zkw_v3.c, Change #define GNSS_TTY and #define GNSS_SPEED to correct value.
3. Open a terminal, and cd to android source path
4. Type command "source build/envsetup.sh" to setup build.
5. Type command "mmm hardware/libgps" to build gps.default.so.
6. Cd to the path(Something like "out/target/product/xxx/system/lib/hw/" where xxx is your platform's name) containing gps.default.so.
7. Connect your device to your computer with USB debug mode.
8. Type command "adb push gps.default.so /system/lib/hw" to install hal driver to your device.

##Framework level

1. Replace GpsStatus.java with the origin one.
2. Open a terminal, and cd to android source path
3. Type command "source build/envsetup.sh" to setup build.
4. Type command "mmm frameworks/base" to build framework.jar
5. Cd to the path(Something like "out/target/product/xxx/system/framework/" where xxx is your platform's name) containing framework.jar
6. Connect your device to your computer with USB debug mode.
8. Type command "adb push framework.jar /system/framework" to update new framework to your device.

##Reboot your device
The driver will take effect after you reboot your device.
