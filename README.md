# Android HAL driver for GPS and BDS
An android HAL driver project, to support for both GPS and BDS satellites system.

##Basic workflow

1. When android system boot, it will auto load /system/hw/lib/gps.default.so
2. gps.default.so first open gps tty and start a thread(gps_state_thread) to listen the gps tty
3. gps_state_thread will alse parse the NMEA protocol on gps tty
4. When the NMEA data containing locaion info(latitude, longitude and altitude), gps_state_thread report location to framework.
5. When the NMEA data containing satellites info(prn, azimuth, elevation, cn0 and is_used), gps_state_thread report satellites's status to framework.

*NMEA protocol is not a topic here

##Android's limited
As android's original location service only support prn from 1 to 32(for it's use an int32 to hold all the satellites's in_use_fix_flag), that's only enough for GPS system. 

To support BDS system, we need something more tricky.

As satellite's azimuth is always in range [0, 360), but with a float type, that's quite enough to hold something more.

So I add in_use_fix_flag to azimuth.

If the satellite is used in fix, it's azimuth will plus 720, else nothing changed.

And when the LocationAPI request satellite's status, if it's azimuth is bigger than 720, then it's used in fix, else, it's not. 

To distinguish GPS and BDS satellites, I use different satellite's id range. [1, 32] for GPS satellites, and [201, 216] for BDS satellites.

As we do all the things in android's hal(parse nmea and conceal satellites's in_use_fix_flag) and framework(reveal satellites's in_use_fix_flag and restore azimuth to normal), so any 3rd party application can work with it.

##Files

1. hardware/libgps/gps_zkw_v3.c
2. hardware/libgps/android.mk
3. frameworks/base/location/java/android/locaton/GpsStatus.java

##Requirements

1. Android source code
2. Android build enviroment.
3. You must first build the full android source.

*How to build android source is not a topic here.

##How to use
###Sepcial notes

1. This project is build and tested on origin android v4.0.4.
2. Someone said they found it also work well on origin android4.4 and android5.x.
3. It definitelly NOT work on any MTK android version.
4. Not guarantee anything on other android versions.

##Framework level

1. Replace android's old /frameworks/base/location/java/android/locaton/GpsStatus.java file with the new one in this project. 
2. Open a terminal, and cd to android source path
3. Type command "source build/envsetup.sh" to setup build.
4. Type command "mmm frameworks/base" to build framework.jar
5. Cd to the path(Something like "out/target/product/xxx/system/framework/" where xxx is your platform's name) containing framework.jar
6. Connect your device to your computer with USB debug mode.
8. Type command "adb push framework.jar /system/framework" to update new framework to your device.

###HAL level

1. Copy folder /hardware/libgps/ to android source path.
2. Edit gps_zkw_v3.c, Change #define GNSS_TTY and #define GNSS_SPEED to correct value.
3. Open a terminal, and cd to android source path.
4. Type command "source build/envsetup.sh" to setup build.
5. Type command "mmm hardware/libgps" to build gps.default.so.
6. Cd to the path(Something like "out/target/product/xxx/system/lib/hw/" where xxx is your platform's name) containing gps.default.so.
7. Connect your device to your computer with USB debug mode.
8. Type command "adb push gps.default.so /system/lib/hw" to install hal driver to your device.

###Taking effect
The driver will taking effect after you reboot the device.

##Snapshots of GPSTest
![alt tag](https://cloud.githubusercontent.com/assets/4736883/21516494/c55fa79c-cd13-11e6-9969-63a8441b13e4.JPG)
* Dots are GPS satellites.
* Triangles are BDS satellites.

![alt tag](https://cloud.githubusercontent.com/assets/4736883/21516497/c98e5c46-cd13-11e6-8c7b-80a699a4f2c3.JPG)
* 10 to 32 with white foreground and black background are GPS satellites.
* 206 with black foreground and white background are BDS satellites.
* The maximum display satellites number of GPSTest are 12, but there are 15 satellites in view, so the rest 3 BDS satellites are not displayed.
