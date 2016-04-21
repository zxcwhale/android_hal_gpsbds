# android_gpsbd
An android hal driver, support for both gps and bds satellites system.

Mainly support for android4.x.

Files:

/hardware/libhardware/include/hardware/gps.h

/hardware/libgps/gps_zkw.c

/hardware/libgps/android.mk

As location service only support prn from 1 to 32, that's only for gps system. 

To support bds satellites, we must pack more infomations, like satellite's system (gps or bds) etc.

Thus I choose the elevation property to hold those infomations.

satellite's elevation is set to: real_prn * 10000 + sv_sys * 1000 + used_in_fix * 100 + real_elevation

and satellite's prn is only a useless number.
