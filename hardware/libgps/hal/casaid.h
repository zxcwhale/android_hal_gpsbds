#ifndef CASAID_H
#define CASAID_H
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "supl.h"

#define BIN_HEADER0						0xBA
#define BIN_HEADER1						0xCE

#define ID_RXM_GPS_EPH					0x0708
#define ID_AID_INI 						0x010B
#define ID_RXM_GPS_UTC					0x0508
#define ID_RXM_GPS_ION					0x0608

#define MAX_EPHEMERIS 32

// 电文有效标志
typedef enum _ME_MESSAGE_FLAG_ENUM
{
  NAVIGATION_MESSAGE_ABSENCE,						// 缺省
  NAVIGATION_MESSAGE_UNHEALTHY,					// 不健康
  NAVIGATION_MESSAGE_OUTOFDATE,					// 过期
  NAVIGATION_MESSAGE_AVAILABLE					// 有效

} ME_MESSAGE_FLAG_ENUM;

// ============================================================
// GPS定点开普勒轨道参数(星历)
typedef struct _GPS_FIX_KEPLER_EPH_STR
{
  unsigned int				sqra;				// 32
  unsigned int				es;					// 32
  int							w;					// 32*
  int							m0;					// 32*
  int							i0;					// 32*
  int							omega0;				// 32*
  int							omegadot;			// 24*
  short int					deltn;				// 16*
  short int					idot;				// 14*
  short int					cuc;				// 16*
  short int					cus;				// 16*
  short int					crc;				// 16*
  short int					crs;				// 16*
  short int					cic;				// 16*
  short int					cis;				// 16*
  unsigned short int			toe;				// 16
  unsigned short int			wne;				// 10

} GPS_FIX_KEPLER_EPH_STR;

// ============================================================
// GPS定点时钟修正参数
typedef struct _GPS_FIX_SV_CLOCK_STR
{
  unsigned int				toc;				// 16
  int							af0;				// 22*
  short int					af1;				// 16*
  signed char					af2;				// 8*
  signed char					tgd;				// 8*

} GPS_FIX_SV_CLOCK_STR;

// ============================================================
// 定点星历
typedef struct _GPS_FIX_EPHEMERIS_STR
{
  unsigned int				wordCheckSum;
  GPS_FIX_KEPLER_EPH_STR		kepler;
  GPS_FIX_SV_CLOCK_STR		svClock;
  unsigned short int			iodc;
  unsigned char				ura;
  unsigned char				health;
  unsigned char				svid;
  unsigned char				valid;
  unsigned short int 			tow;				// 12秒

} GPS_FIX_EPHEMERIS_STR;

// ============================================================
// 定点UTC(GPS和BD2采用相同的信息格式)
typedef struct _FIX_UTC_STR
{
  unsigned int			wordCheckSum;
  int						a0;						// 32*
  int						a1;						// 24*
  signed char				dtls;					// 8*
  signed char				dtlsf;					// 8*
  unsigned char			tot;					// 8
  unsigned char			wnt;					// 8
  unsigned char			wnlsf;					// 8
  unsigned char			dn;						// 8
  unsigned char			valid;

} FIX_UTC_STR;

// ============================================================
// 8参数定点电离层(GPS卫星和BD2卫星)
typedef struct _FIX_IONO_STR
{
  unsigned int			wordCheckSum;
  signed char				alpha0;					// 8*
  signed char				alpha1;					// 8*
  signed char				alpha2;					// 8*
  signed char				alpha3;					// 8*
  signed char				beta0;					// 8*
  signed char				beta1;					// 8*
  signed char				beta2;					// 8*
  signed char				beta3;					// 8*
  unsigned char			valid;

} FIX_IONO_STR;

// 辅助信息（位置，时间，频率）
typedef struct  _AID_INI_STR
{
  double							xOrLat;
  double							yOrLon;
  double							zOrAlt;
  double							tow;
  float							df;
  float							posAcc;
  float							tAcc;
  float							fAcc;
  unsigned int					res;
  unsigned short int				wn;
  unsigned char					timeSource;
  unsigned char					flags;

} AID_INI_STR;

typedef struct
{
  double							xOrLat, yOrLon, zOrAlt;
  float							df;
  float							posAcc;
  float							tAcc;
  float							fAcc;
  int								gnssMask;
  int								posFlags;	// 0: 无效，1: ECEF坐标系 2: LLA坐标系
  int								reqMode;	// 0: 全部信息, 1: 可见信息
  int								reqMask;
  int								year;
  int								month;
  int								day;
  int								hour;
  int								minute;
  int								second;
  float							ms;

} AID_REQ_STR;

unsigned int cas_make_msg(int id, int *msg, int n, unsigned char *buff);
void supl2cas_ini(supl_assist_t *ctx, AID_INI_STR *cas_int);
void supl2cas_eph(unsigned short wn, struct supl_ephemeris_s *eph_ctx, GPS_FIX_EPHEMERIS_STR *cas_eph);
void supl2cas_utc(struct supl_utc_s *utc_ctx, FIX_UTC_STR *cas_utc);
void supl2cas_iono(struct supl_ionospheric_s *iono_ctx, FIX_IONO_STR *cas_iono);
#endif
