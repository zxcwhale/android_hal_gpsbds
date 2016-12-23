中科微HAL层驱动重大修改
程序流程
1.正常解析NMEA语句,使用卫星编号偏移量标志卫星系统: GPS(1-32), 北斗(201-216)
2.避开系统对于卫星是否参与定位的int32标志位.
3.在gps_zkw_v3.c的encode_sv_status函数中, 将参与定位的卫星的方位角(azimuth)加上720度的偏移.
3.在GpsStatus的setStatus函数中, 如果卫星的方位角大于720, 那么该卫星参与定位,并将方位角复原.

编译方式
0.!!!安装前请做好设备上的文件备份!!!
1.建立环境: source build/envsetup.sh
2.编译gps_zkw_v3.c: mmm hardware/libgps
4.安装gps.default.so到设备/system/lib/hw/
3.编译GpsStatus.java: mmm frameworks/base 
5.安装framework.jar到设备/system/framework


