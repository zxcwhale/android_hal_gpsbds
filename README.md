# android_hal_gpsbds
An android hal driver, support for both gps and bds satellites system.

Mainly support for android4.x and 5.x

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
