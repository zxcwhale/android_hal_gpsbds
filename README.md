# android_hal_gpsbds
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

In an java app, the parse code will be like this

@Override

public void onGpsStatusChanged(int i) {

	GpsStatus status=lm.getGpsStatus(null);

	Iterator<GpsSatellite> it= status.getSatellites().iterator();

	while(it.hasNext()){

		GpsSatellite sv = it.next();

		int fakeElev = (int)sv.getElevation();
		int prn = fakeElev / 10000;		// get real prn
		fakeElev -= prn * 10000;
		int sys = fakeElev / 1000;		// get sv system flag
		fakeElev -= sys * 1000;
		int used = fakeElev / 100;		// get is used in fix flag
		int realElev = fakeElev % 100;	// get read elevation

		MySate sate = new MySate();	
		sate.prn = prn;				
		sate.elev = realElev; 	
		sate.azim = sv.getAzimuth();	
		sate.snr = sv.getSnr();		
		if (sys == 0)					
			sate.sys = “gps”;
		else if (sys == 1)
			sate.sys = “bd”;
		if (used == 1)				
			sate.inUse = true;
		else
			sate.inUse = false;
}
