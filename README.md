# Android HAL driver for GPS and BDS
This is an GNSS HAL driver project. Support for both GPS and BDS satellites system.
And use SUPL2.0 for AGPS.

## Basics
1. This project is about to make a gps.xxxx.so lib, which loaded by android system at the booting time.
2. When gps.xxxx.so is loaded, it start an new thread to listen the gnss tty.
3. The gnss device outputs navigation data in the format of [NMEA 0183 protocol](https://en.wikipedia.org/wiki/NMEA_0183) to gnss tty.
4. The gps.xxxx.so lib draw location information and satellite's status from navigation data.
5. The gps.xxxx.so lib contains some "callback" functions, which used to report location information and satellite's status to android framework.

## A little trick

As android's original location service only support prn from 1 to 32(for it's use an int32 to hold all the satellites's in_use_fix_flag), that's only enough for GPS system. 

To support BDS system, we need something more tricky.

As satellite's azimuth is always in range [0, 360), but with a float type, that's quite enough to hold something more.

So I add in_use_fix_flag to azimuth.

If the satellite is used in fix, it's azimuth will plus 720, else nothing changed.

And when the LocationAPI request satellite's status, if it's azimuth is bigger than 720, then it's used in fix, else, it's not. 

I use different satellite's id range to distinguish GPS and BDS satellites.  [1, 32] for GPS satellites, and [201, 216] for BDS satellites.

As we do all the things in android's hal(parse nmea and conceal satellites's in_use_fix_flag) and framework(reveal satellites's in_use_fix_flag and restore azimuth to normal), so any 3rd party application can work with it.

## Requirements

1. Android source code.
2. Android build enviroment.
3. [Build the android source](https://source.android.com/source/requirements.html).


## Sepcial notes

1. This project is build and tested on origin android v4.0.4.
2. Someone said they found it also work well on origin android4.4 and android5.x.
3. It definitelly NOT work on any MTK android version.
4. Not guarantee anything on other android versions.

## How to use

1. Replace android's old /frameworks/base/location/java/android/locaton/GpsStatus.java file with the new one in this project. 
2. Copy folder /hardware/libgps/ to android source path.
3. Rebuild android source code.
4. Update your device with the new image.
5. Change the settings in /system/etc/gnss.conf and copy it to android device.

## Snapshots of GPSTest
![alt tag](https://cloud.githubusercontent.com/assets/4736883/21558868/1b6a6fc8-ce7c-11e6-9251-ef4aa9781d4d.png)

* Dots are GPS satellites.
* Triangles are BDS satellites.

![alt tag](https://cloud.githubusercontent.com/assets/4736883/21558867/1b691146-ce7c-11e6-93fb-ec7dd9784f20.png)
* 02,05,13,15,20,21,24,29 and 30 with white foreground and black background are GPS satellites.
* 201, 202 and 203 with black foreground and white background are BDS satellites.
* The maximum display satellites number of GPSTest are 12, but there are 21 satellites in view, so the rest are not displayed.


## SUPL2

1. SUPL2 is an optional feature, which can improves the gnss devices's startup performance.
2. All my SUPL codes are based on [tajuma's project](https://github.com/tajuma/supl).
3. To support LTE(4G) network, I update asn-rrlp and asn-supl to SUPL2.0.
4. To enable/disable SUPL2, edit hardware/libgps/Android.mk and set SUPL\_ENABLED := 1/0.
