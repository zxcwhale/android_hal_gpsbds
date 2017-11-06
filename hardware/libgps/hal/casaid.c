#include <stdlib.h>
#include <string.h>
#include "casaid.h"

unsigned int msgCheckSum(unsigned int data[], int n)
{
	int i;
	unsigned int sum = 0;

	for (i = n - 1; i != 0; i--)
	{
		sum += data[i];
	}

	return sum;
}

unsigned int msgPacketSend(int id, int *pMsg, int n, unsigned char *pSendMsg)
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
		ckSum += pMsg[i];
	}
	// 
	memcpy(pSendMsg, head, 6);
	memcpy(pSendMsg + 6, (char *)pMsg, n);
	memcpy(pSendMsg + 6 + n, (char *)(&ckSum), 4);

	return (n + 10);
}

void supl2casicIni(supl_assist_t *pSupltp, AID_INI_STR *pCasicIni)
{
	pCasicIni->flags		= 0x00;

	if (pSupltp->set & SUPL_RRLP_ASSIST_REFTIME) {
		pCasicIni->tow			= pSupltp->time.gps_tow * 0.08;
		pCasicIni->wn			= pSupltp->time.gps_week + 1024;
		pCasicIni->flags		|= 0x02;
	}

	if (pSupltp->set & SUPL_RRLP_ASSIST_REFLOC) {
		pCasicIni->xOrLat		= pSupltp->pos.lat;
		pCasicIni->yOrLon		= pSupltp->pos.lon;
		pCasicIni->zOrAlt		= 0;
		pCasicIni->posAcc		= 5000;
		//pCasicIni->posAcc		= pSupltp->pos.uncertainty;
		//pCasicIni->posAcc		= 10.0*(pow(1.1, pSupltp->pos.uncertainty)-1);
		pCasicIni->flags		|= 0x21;
	}

	pCasicIni->timeSource	= 0;

}

void supl2casicEph(unsigned short wn, struct supl_ephemeris_s *pSupl, GPS_FIX_EPHEMERIS_STR *pCasic)
{
	pCasic->ura				= pSupl->ura;
	pCasic->svid			= pSupl->prn;
	pCasic->iodc			= pSupl->IODC;
	pCasic->kepler.sqra		= pSupl->A_sqrt;
	pCasic->kepler.es		= pSupl->e;
	pCasic->kepler.m0		= pSupl->M0;
	pCasic->kepler.i0		= pSupl->i0;
	pCasic->kepler.idot		= pSupl->i_dot;
	pCasic->kepler.omega0	= pSupl->OMEGA_0;
	pCasic->kepler.omegadot	= pSupl->OMEGA_dot;
	pCasic->kepler.w		= pSupl->w;
	pCasic->kepler.deltn	= pSupl->delta_n;
	pCasic->kepler.cic		= pSupl->Cic;
	pCasic->kepler.cis		= pSupl->Cis;
	pCasic->kepler.crc		= pSupl->Crc;
	pCasic->kepler.crs		= pSupl->Crs;
	pCasic->kepler.cuc		= pSupl->Cuc;
	pCasic->kepler.cus		= pSupl->Cus;
	pCasic->kepler.toe		= pSupl->toe;
	pCasic->kepler.wne		= wn;
	//
	pCasic->svClock.toc		= pSupl->toc;
	pCasic->svClock.af0		= pSupl->AF0;
	pCasic->svClock.af1		= pSupl->AF1;
	pCasic->svClock.af2		= pSupl->AF2;
	pCasic->svClock.tgd		= pSupl->tgd;
	//
	pCasic->health			= pSupl->health;
	if (pCasic->health == 0)
	{
		pCasic->valid		= NAVIGATION_MESSAGE_AVAILABLE;
	}
	else
	{
		pCasic->valid		= NAVIGATION_MESSAGE_UNHEALTHY;
	}
	pCasic->wordCheckSum	= msgCheckSum((unsigned int *)pCasic, sizeof(GPS_FIX_EPHEMERIS_STR));
}

void supl2casicUtc(struct supl_utc_s *pSupl, FIX_UTC_STR *pCasic)
{
	pCasic->a0				= pSupl->a0;
	pCasic->a1				= pSupl->a1;
	pCasic->dn				= pSupl->dn;
	pCasic->dtls			= pSupl->delta_tls;
	pCasic->dtlsf			= pSupl->delta_tlsf;
	pCasic->tot				= pSupl->tot;
	pCasic->wnlsf			= pSupl->wnlsf;
	pCasic->wnt				= pSupl->wnt;
	pCasic->valid			= NAVIGATION_MESSAGE_AVAILABLE;
	pCasic->wordCheckSum	= msgCheckSum((unsigned int *)pCasic, sizeof(FIX_UTC_STR));
}

void supl2casicIon(struct supl_ionospheric_s *pSupl, FIX_IONO_STR *pCasic)
{
	pCasic->alpha0			= pSupl->a0;
	pCasic->alpha1			= pSupl->a1;
	pCasic->alpha2			= pSupl->a2;
	pCasic->alpha3			= pSupl->a3;

	pCasic->beta0			= pSupl->b0;
	pCasic->beta1			= pSupl->b1;
	pCasic->beta2			= pSupl->b2;
	pCasic->beta3			= pSupl->b3;
	pCasic->valid			= NAVIGATION_MESSAGE_AVAILABLE;
	pCasic->wordCheckSum	= msgCheckSum((unsigned int *)pCasic, sizeof(FIX_IONO_STR));
}


/*
void main(AID_REQ_STR *pAidReq, supl_assist_t *pSupldat, unsigned char *pAidData)
{
	GPS_FIX_EPHEMERIS_STR uTempGpsEph;
	AID_INI_STR uTempAidIni;
	FIX_UTC_STR uTempUtc;
	FIX_IONO_STR uTempIon;

	int ch;
	unsigned int aidDataLen;
	int reqMaskinfo;
	int gpsEphEn = 0;
	int bd2EphEn = 0;
	int glnEphEn = 0;
	int gpsUtcEn = 0;
	int bd2UtcEn = 0;
	int gpsIonEn = 0;
	int bd2IonEn = 0;
	int aidIniEn = 0;

	reqMaskinfo = pAidReq->reqMask;

	switch(pAidReq->gnssMask)
	{
	case 1: reqMaskinfo &= 0x0223; break;
	case 2: reqMaskinfo &= 0x0445; break;
	case 3: reqMaskinfo &= 0x0667; break;
	case 4: reqMaskinfo &= 0x0009; break;
	case 5: reqMaskinfo &= 0x022B; break;
	case 7: reqMaskinfo &= 0x066F; break;
	default: break;
	}

	aidIniEn = (reqMaskinfo & 0x001);
	gpsEphEn = (reqMaskinfo & 0x002);
	bd2EphEn = (reqMaskinfo & 0x004);
	glnEphEn = (reqMaskinfo & 0x008);
	gpsUtcEn = (reqMaskinfo & 0x020);
	bd2UtcEn = (reqMaskinfo & 0x040);
	gpsIonEn = (reqMaskinfo & 0x200);
	bd2IonEn = (reqMaskinfo & 0x400);

	//-----------------------------------------------------------------------------------------------------------------
	// 开始构建发送数据包
	aidDataLen = 0;
	// 辅助定位的位置时间辅助
	supl2casicIni(pSupldat, &uTempAidIni);
	if (((uTempAidIni.flags & 0x3) != 0) && (aidIniEn == 1))
	{
		aidDataLen += msgPacketSend(ID_AID_INI, 		(int *)(&uTempAidIni), 	sizeof(uTempAidIni),			pAidData + aidDataLen);
	}
	// GPS星历
	for (ch = 0; ch < MAX_EPHEMERIS; ch++)
	{
		supl2casicEph((unsigned short)pSupldat->time.gps_week, &pSupldat->eph[ch], &uTempGpsEph);
		if ((uTempGpsEph.valid != NAVIGATION_MESSAGE_AVAILABLE) || (gpsEphEn == 0))	continue;
		aidDataLen += msgPacketSend(ID_RXM_GPS_EPH, 	(int *)(&uTempGpsEph),	sizeof(GPS_FIX_EPHEMERIS_STR),	pAidData + aidDataLen);
	}
	// GPS的UTC参数
	supl2casicUtc(&pSupldat->utc, &uTempUtc);
	if ((uTempUtc.valid == NAVIGATION_MESSAGE_AVAILABLE) && (gpsUtcEn))
	{
		aidDataLen += msgPacketSend(ID_RXM_GPS_UTC, 	(int *)(&uTempUtc),		sizeof(FIX_UTC_STR),			pAidData + aidDataLen);
	}
	// GPS的电离层参数
	supl2casicIon(&pSupldat->iono, &uTempIon);
	if ((uTempIon.valid == NAVIGATION_MESSAGE_AVAILABLE) && (gpsIonEn))
	{
		aidDataLen += msgPacketSend(ID_RXM_GPS_ION, 	(int *)(&uTempIon),		sizeof(FIX_IONO_STR),			pAidData + aidDataLen);
	}

	
	
	
}
*/
