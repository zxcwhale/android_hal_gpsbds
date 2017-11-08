#include <stdlib.h>
#include <string.h>
#include "casaid.h"

unsigned int calc_checksum(unsigned int data[], int n)
{
  int i;
  unsigned int sum = 0;

  for (i = n - 1; i != 0; i--)
  {
    sum += data[i];
  }

  return sum;
}

unsigned int cas_make_msg(int id, int *msg, int n, unsigned char *buff)
{
  int i;
  int ckSum;
  char head[6];

  head[0] = 0xBA;
  head[1] = 0xCE;
  head[2] = (char)(n & 0xFF);		// LENGTH
  head[3] = (char)(n >> 8);
  head[4] = (char)(id & 0xFF);	// CLASS	ID
  head[5] = (char)(id >> 8);		// MESSAGE	ID

  // 32-bit Sum Algorithm
  //
  ckSum = (id << 16) + n;
  for (i = 0; i < (n / 4); i++)
  {
    ckSum += msg[i];
  }
  //
  memcpy(buff, head, 6);
  memcpy(buff + 6, (char *)msg, n);
  memcpy(buff + 6 + n, (char *)(&ckSum), 4);

  return (n + 10);
}

void supl2cas_ini(supl_assist_t *ctx, AID_INI_STR *cas_ini)
{
  cas_ini->flags		= 0x00;

  if (ctx->set & SUPL_RRLP_ASSIST_REFTIME) {
    cas_ini->tow	  = ctx->time.gps_tow * 0.08;
    cas_ini->wn			= ctx->time.gps_week + 1024;
    cas_ini->flags  |= 0x02;
  }

  if (ctx->set & SUPL_RRLP_ASSIST_REFLOC) {
    cas_ini->xOrLat		= ctx->pos.lat;
    cas_ini->yOrLon		= ctx->pos.lon;
    cas_ini->zOrAlt		= 0;
    cas_ini->posAcc		= 5000;
    //cas_ini->posAcc		= ctx->pos.uncertainty;
    //cas_ini->posAcc		= 10.0*(pow(1.1, ctx->pos.uncertainty)-1);
    cas_ini->flags		|= 0x21;
  }

  cas_ini->timeSource	= 0;

}

void supl2cas_eph(unsigned short wn, struct supl_ephemeris_s *eph_ctx, GPS_FIX_EPHEMERIS_STR *cas_eph)
{
  cas_eph->ura				  = eph_ctx->ura;
  cas_eph->svid			    = eph_ctx->prn;
  cas_eph->iodc			    = eph_ctx->IODC;
  cas_eph->kepler.sqra	= eph_ctx->A_sqrt;
  cas_eph->kepler.es		= eph_ctx->e;
  cas_eph->kepler.m0		= eph_ctx->M0;
  cas_eph->kepler.i0		= eph_ctx->i0;
  cas_eph->kepler.idot  = eph_ctx->i_dot;
  cas_eph->kepler.omega0	  = eph_ctx->OMEGA_0;
  cas_eph->kepler.omegadot	= eph_ctx->OMEGA_dot;
  cas_eph->kepler.w		  = eph_ctx->w;
  cas_eph->kepler.deltn	= eph_ctx->delta_n;
  cas_eph->kepler.cic		= eph_ctx->Cic;
  cas_eph->kepler.cis		= eph_ctx->Cis;
  cas_eph->kepler.crc		= eph_ctx->Crc;
  cas_eph->kepler.crs		= eph_ctx->Crs;
  cas_eph->kepler.cuc		= eph_ctx->Cuc;
  cas_eph->kepler.cus		= eph_ctx->Cus;
  cas_eph->kepler.toe		= eph_ctx->toe;
  cas_eph->kepler.wne		= wn;
  //
  cas_eph->svClock.toc	= eph_ctx->toc;
  cas_eph->svClock.af0	= eph_ctx->AF0;
  cas_eph->svClock.af1	= eph_ctx->AF1;
  cas_eph->svClock.af2	= eph_ctx->AF2;
  cas_eph->svClock.tgd	= eph_ctx->tgd;
  //
  cas_eph->health			  = eph_ctx->health;
  if (cas_eph->health == 0)
  {
    cas_eph->valid = NAVIGATION_MESSAGE_AVAILABLE;
  }
  else
  {
    cas_eph->valid = NAVIGATION_MESSAGE_UNHEALTHY;
  }
  cas_eph->wordCheckSum	= calc_checksum((unsigned int *)cas_eph, sizeof(GPS_FIX_EPHEMERIS_STR) / 4);
}

void supl2cas_utc(struct supl_utc_s *utc_ctx, FIX_UTC_STR *cas_utc)
{
  cas_utc->a0				= utc_ctx->a0;
  cas_utc->a1				= utc_ctx->a1;
  cas_utc->dn				= utc_ctx->dn;
  cas_utc->dtls			= utc_ctx->delta_tls;
  cas_utc->dtlsf			= utc_ctx->delta_tlsf;
  cas_utc->tot				= utc_ctx->tot;
  cas_utc->wnlsf			= utc_ctx->wnlsf;
  cas_utc->wnt				= utc_ctx->wnt;
  cas_utc->valid			= NAVIGATION_MESSAGE_AVAILABLE;
  cas_utc->wordCheckSum	= calc_checksum((unsigned int *)cas_utc, sizeof(FIX_UTC_STR) / 4);
}

void supl2cas_iono(struct supl_ionospheric_s *iono_ctx, FIX_IONO_STR *cas_iono)
{
  cas_iono->alpha0			= iono_ctx->a0;
  cas_iono->alpha1			= iono_ctx->a1;
  cas_iono->alpha2			= iono_ctx->a2;
  cas_iono->alpha3			= iono_ctx->a3;

  cas_iono->beta0			= iono_ctx->b0;
  cas_iono->beta1			= iono_ctx->b1;
  cas_iono->beta2			= iono_ctx->b2;
  cas_iono->beta3			= iono_ctx->b3;
  cas_iono->valid			= NAVIGATION_MESSAGE_AVAILABLE;
  cas_iono->wordCheckSum	= calc_checksum((unsigned int *)cas_iono, sizeof(FIX_IONO_STR) / 4);
}
