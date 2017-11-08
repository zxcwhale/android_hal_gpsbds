/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* this implements a GPS hardware library for the Android emulator.
 * the following code should be built as a shared library that will be
 * placed into /system/lib/hw/gps.goldfish.so
 *
 * it will be loaded by the code in hardware/libhardware/hardware.c
 * which is itself called from android_location_GpsLocationProvider.cpp
 */


#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <math.h>
#include <time.h>
#include <netinet/in.h>
#include <netdb.h>

#define  LOG_TAG  "gps_zkw"
#include <cutils/log.h>
#include <cutils/sockets.h>
#include <hardware/gps.h>
#include <cutils/properties.h>

#ifdef SUPL_ENABLED
#include "supl.h"
#include "casaid.h"
#endif
/* the name of the qemud-controlled socket */

#define GPS_DEBUG  1
#define NMEA_DEBUG 0
#define SUPL_TEST 0
#define GPS_SV_INCLUDE 1

typedef enum {
  GPS_SV = 0,
  BDS_SV = 1,
  GLONASS_SV = 2
} SV_TYPE;

#define PRN_PLUS_BDS 200
#define PRN_PLUS_GLN 64

//#define GNSS_TTY "/dev/ttySAC0"
//#define GNSS_SPEED B9600

#if GPS_DEBUG
#  define  D(f, ...)   LOGD("%s: line = %d, " f, __func__, __LINE__, ##__VA_ARGS__)
#else
#  define  D(...)   ((void)0)
#endif


static char tty_name[16] = "/dev/ttyGNSS";
static int tty_baud = B9600;
static char supl_host[64] = "supl.qxwz.com";
static char supl_port[16] = "7275";

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       G N S S . C O N F   L O A D E R                 *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

static void 
remove_comments(char *s) {
  int i;
  for (i = 0; s[i] != 0; i++) {
    if (s[i] == '#') {
      s[i] = 0;
      break;
    }
  }
}

static int 
is_space(char c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

static void 
remove_space(char *s) {
  int i = 0; 
  int j = 0;
  while( s[i] != '\0') {
    if (!is_space(s[i])) {
      s[j] = s[i];
      j = j + 1;
    }
    i = i + 1;
  }
  s[j] = '\0';
  //for (i=0, j=0; s[j]=s[i]; j+=!is_space(s[i++]));
}

static int 
int2baud(int n) {
  switch(n) {
    case 4800:
      return B4800;
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    default:
      return 0;
  }
}

void 
load_conf() {
  FILE *fp;
  char line[256];
  int i;

  fp = fopen("/system/etc/gnss.conf", "r");
  if (fp == NULL) {
    D("Can not open gnss.conf");
    return;
  }
  while (fgets(line, sizeof(line), fp) != NULL) {
    remove_comments(line);
    remove_space(line);
    if (line[0] != 0) {
      //printf("Redline: %s. %d\n", line, is_space('\n'));
      char *splitter = "=";
      char *key; 
      char *value;
      key = strtok(line, splitter);
      value = strtok(NULL, splitter);       
      // printf("Key=%s, value=%s\n", key, value);
      if (key != NULL && value != NULL) {
        if (strcmp(key, "TTY_NAME") == 0) {
          memset(tty_name, 0, sizeof(tty_name));
          strncpy(tty_name, value, sizeof(tty_name) - 1);
          D("Load tty name: %s\n", tty_name);
        } else if (strcmp(key, "TTY_BAUD") == 0) {
          int temp = 0;
          sscanf(value, "%d", &temp);
          temp = int2baud(temp);
          if (temp) tty_baud = temp;
          D("Load tty baud: %d\n", tty_baud);
        } else if (strcmp(key, "SUPL_HOST") == 0) {
          memset(supl_host, 0, sizeof(supl_host));
          strncpy(supl_host, value, sizeof(supl_host) - 1);
          D("Load supl host: %s\n", supl_host);
        } else if (strcmp(key, "SUPL_PORT") == 0) {
          memset(supl_port, 0, sizeof(supl_port));
          strncpy(supl_port, value, sizeof(supl_port) - 1);
          D("Load supl port: %s\n", supl_port);
        }
      }
    }
  }

  fclose(fp);
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   T O K E N I Z E R                     *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

typedef struct {
  const char*  p;
  const char*  end;
} Token;

#define  MAX_NMEA_TOKENS  32

typedef struct {
  int     count;
  Token   tokens[ MAX_NMEA_TOKENS ];
} NmeaTokenizer;

static int
nmea_tokenizer_init( NmeaTokenizer*  t, const char*  p, const char*  end )
{
  int    count = 0;
  char*  q;

  // the initial '$' is optional
  if (p < end && p[0] == '$')
    p += 1;

  // remove trailing newline
  if (end > p && end[-1] == '\n') {
    end -= 1;
    if (end > p && end[-1] == '\r')
      end -= 1;
  }

  // get rid of checksum at the end of the sentecne
  if (end >= p+3 && end[-3] == '*') {
    end -= 3;
  }

  while (p < end) {
    const char*  q = p;

    q = memchr(p, ',', end-p);
    if (q == NULL)
      q = end;
    // if (q > p) {
    // q >= p include empty token: ,,
    if (q >= p) {
      if (count < MAX_NMEA_TOKENS) {
        t->tokens[count].p   = p;
        t->tokens[count].end = q;
        count += 1;
      }
    }
    if (q < end)
      q += 1;

    p = q;
  }

  t->count = count;
  return count;
}

static Token
nmea_tokenizer_get( NmeaTokenizer*  t, int  index )
{
  Token  tok;
  static const char*  dummy = "";

  if (index < 0 || index >= t->count) {
    tok.p = tok.end = dummy;
  } else
    tok = t->tokens[index];

  return tok;
}


static int
str2int( const char*  p, const char*  end )
{
  int   result = 0;
  int   len    = end - p;

  for ( ; len > 0; len--, p++ )
  {
    int  c;

    if (p >= end)
      goto Fail;

    c = *p - '0';
    if ((unsigned)c >= 10)
      goto Fail;

    result = result*10 + c;
  }
  return  result;

Fail:
  return -1;
}

static double
str2float( const char*  p, const char*  end )
{
  int   result = 0;
  int   len    = end - p;
  char  temp[16];

  if (len >= (int)sizeof(temp))
    return 0.;

  memcpy( temp, p, len );
  temp[len] = 0;
  return strtod( temp, NULL );
}

#ifdef SUPL_ENABLED
static time_t last_supl_time = 0;
static supl_ctx_t supl_ctx;
static AGpsRilCallbacks *agpsRilCallbacks;
void
agps_ril_init (AGpsRilCallbacks *callbacks) {
  agpsRilCallbacks = callbacks;
}

void
agps_ril_set_ref_location (const AGpsRefLocation *agps_reflocation, size_t sz_struct) {
  D("type = %d, mcc = %d, mnc = %d, lac = %d, cid = %d",
    agps_reflocation->u.cellID.type,
    agps_reflocation->u.cellID.mcc,
    agps_reflocation->u.cellID.mnc,
    agps_reflocation->u.cellID.lac,
    agps_reflocation->u.cellID.cid
   );
  if (agps_reflocation->type == AGPS_REF_LOCATION_TYPE_GSM_CELLID) {
    if (agps_reflocation->u.cellID.cid < 65536) {
      supl_set_gsm_cell(&supl_ctx, agps_reflocation->u.cellID.mcc,
                        agps_reflocation->u.cellID.mnc,
                        agps_reflocation->u.cellID.lac,
                        agps_reflocation->u.cellID.cid);
    } else {
      supl_set_lte_cell(&supl_ctx, agps_reflocation->u.cellID.mcc,
                        agps_reflocation->u.cellID.mnc,
                        agps_reflocation->u.cellID.lac,
                        agps_reflocation->u.cellID.cid,
                        0);
    }
  }
  else if (agps_reflocation->type == AGPS_REF_LOCATION_TYPE_UMTS_CELLID) {
    supl_set_wcdma_cell(&supl_ctx, agps_reflocation->u.cellID.mcc,
                        agps_reflocation->u.cellID.mcc,
                        agps_reflocation->u.cellID.cid);
  }
  else {
    D("No cell info");
  }
}

void
agps_ril_set_set_id (AGpsSetIDType type, const char* setid) {
  D("type = %d, setid = %s", type, setid);
  if (type == AGPS_SETID_TYPE_MSISDN) {
    D("set msisdn");
    supl_set_msisdn(&supl_ctx, setid);
  }
}

static const AGpsRilInterface zkwAGpsRilInterface = {
  .size = sizeof(AGpsRilInterface),
  .init = agps_ril_init,
  .set_ref_location = agps_ril_set_ref_location,
  .set_set_id = agps_ril_set_set_id,
  .ni_message = NULL,
  .update_network_state = NULL,
  .update_network_availability = NULL
};

#endif

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   P A R S E R                           *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

#define  NMEA_MAX_SIZE  83
#define  MAX_SV_PRN 256
typedef struct {
  int     pos;
  int     overflow;
  int     utc_year;
  int     utc_mon;
  int     utc_day;
  int     utc_diff;
  GpsLocation  fix;
  GpsStatus status;
#if GPS_SV_INCLUDE
  GpsSvStatus  sv_status;
  int    sv_num;
  int     sv_status_changed;
  char  sv_used_in_fix[MAX_SV_PRN];
#endif
  gps_location_callback  callback;
  gps_nmea_callback nmea_callback;
  gps_status_callback status_callback;
#if GPS_SV_INCLUDE
  gps_sv_status_callback sv_callback;
#endif
  char    in[ NMEA_MAX_SIZE+1 ];
} NmeaReader;


static void
nmea_reader_update_utc_diff( NmeaReader*  r )
{
  time_t         now = time(NULL);
  struct tm      tm_local;
  struct tm      tm_utc;
  long           time_local, time_utc;

  gmtime_r( &now, &tm_utc );
  localtime_r( &now, &tm_local );

  time_local = tm_local.tm_sec +
               60*(tm_local.tm_min +
                   60*(tm_local.tm_hour +
                       24*(tm_local.tm_yday +
                           365*tm_local.tm_year)));

  time_utc = tm_utc.tm_sec +
             60*(tm_utc.tm_min +
                 60*(tm_utc.tm_hour +
                     24*(tm_utc.tm_yday +
                         365*tm_utc.tm_year)));

  r->utc_diff = time_local - time_utc;
}


static void
nmea_reader_init( NmeaReader*  r )
{
  memset( r, 0, sizeof(*r) );

  r->pos      = 0;
  r->overflow = 0;
  r->utc_year = -1;
  r->utc_mon  = -1;
  r->utc_day  = -1;
  r->callback = NULL;
  r->nmea_callback = NULL;
  r->status_callback = NULL;
  r->sv_status.num_svs = 0;
  r->fix.size = sizeof(GpsLocation);

  nmea_reader_update_utc_diff( r );
}

static void
nmea_reader_set_nmea_callback( NmeaReader* r, gps_nmea_callback cb)
{
  r->nmea_callback = cb;
  if(cb != NULL) {
    D("Sending nmea to new callback");
  }
}

static void
nmea_reader_set_status_callback( NmeaReader* r, gps_status_callback cb)
{
  r->status_callback = cb;
  if(cb != NULL) {
    D("Sending status to new callback");
  }
}

static void
nmea_reader_set_callback( NmeaReader*  r, gps_location_callback  cb )
{
  r->callback = cb;
  if (cb != NULL && r->fix.flags != 0) {
    D("Sending latest fix to new callback");
    r->callback( &r->fix );
    r->fix.flags = 0;
  }
}

#if GPS_SV_INCLUDE
static void
nmea_reader_set_sv_callback( NmeaReader*  r, gps_sv_status_callback  cb )
{
  r->sv_callback = cb;
  if (cb != NULL) {
    D("Sending latest sv info to new callback");
  }
}
#endif

static int
nmea_reader_update_time( NmeaReader*  r, Token  tok )
{
  int        hour, minute;
  double     seconds;
  struct tm  tm;
  time_t     fix_time;

  if (tok.p + 6 > tok.end)
    return -1;

  if (r->utc_year < 0) {
    // no date yet, get current one
    time_t  now = time(NULL);
    gmtime_r( &now, &tm );
    r->utc_year = tm.tm_year + 1900;
    r->utc_mon  = tm.tm_mon + 1;
    r->utc_day  = tm.tm_mday;
  }

  hour    = str2int(tok.p,   tok.p+2);
  minute  = str2int(tok.p+2, tok.p+4);
  seconds = str2float(tok.p+4, tok.end);

  tm.tm_hour  = hour;
  tm.tm_min   = minute;
  tm.tm_sec   = (int) seconds;
  tm.tm_year  = r->utc_year - 1900;
  tm.tm_mon   = r->utc_mon - 1;
  tm.tm_mday  = r->utc_day;
  tm.tm_isdst = -1;

  fix_time = mktime( &tm ) + r->utc_diff;
  r->fix.timestamp = (long long)fix_time * 1000;
  return 0;
}

static int
nmea_reader_update_date( NmeaReader*  r, Token  date, Token  time )
{
  Token  tok = date;
  int    day, mon, year;

  if (tok.p + 6 != tok.end) {
    D("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
    return -1;
  }
  day  = str2int(tok.p, tok.p+2);
  mon  = str2int(tok.p+2, tok.p+4);
  year = str2int(tok.p+4, tok.p+6) + 2000;

  if ((day|mon|year) < 0) {
    D("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
    return -1;
  }

  r->utc_year  = year;
  r->utc_mon   = mon;
  r->utc_day   = day;

  return nmea_reader_update_time( r, time );
}


static double
convert_from_hhmm( Token  tok )
{
  double  val     = str2float(tok.p, tok.end);
  int     degrees = (int)(floor(val) / 100);
  double  minutes = val - degrees * 100.;
  double  dcoord  = degrees + minutes / 60.0;
  return dcoord;
}


static int
nmea_reader_update_latlong( NmeaReader*  r,
                            Token        latitude,
                            char         latitudeHemi,
                            Token        longitude,
                            char         longitudeHemi )
{
  double   lat, lon;
  Token    tok;

  tok = latitude;
  if (tok.p + 6 > tok.end) {
    D("latitude is too short: '%.*s'", tok.end-tok.p, tok.p);
    return -1;
  }
  lat = convert_from_hhmm(tok);
  if (latitudeHemi == 'S')
    lat = -lat;

  tok = longitude;
  if (tok.p + 6 > tok.end) {
    D("longitude is too short: '%.*s'", tok.end-tok.p, tok.p);
    return -1;
  }
  lon = convert_from_hhmm(tok);
  if (longitudeHemi == 'W')
    lon = -lon;

  r->fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
  r->fix.latitude  = lat;
  r->fix.longitude = lon;
  return 0;
}


static int
nmea_reader_update_altitude( NmeaReader*  r,
                             Token        altitude,
                             Token        units )
{
  double  alt;
  Token   tok = altitude;

  if (tok.p >= tok.end)
    return -1;

  r->fix.flags   |= GPS_LOCATION_HAS_ALTITUDE;
  r->fix.altitude = str2float(tok.p, tok.end);
  return 0;
}

static int
nmea_reader_update_accuracy( NmeaReader*  r,
                             Token        accuracy )
{
  double  acc;
  Token   tok = accuracy;

  if (tok.p >= tok.end)
    return -1;

  r->fix.accuracy = str2float(tok.p, tok.end);

  if (r->fix.accuracy == 99.99) {
    return 0;
  }

  r->fix.flags   |= GPS_LOCATION_HAS_ACCURACY;
  return 0;
}

static int
nmea_reader_update_bearing( NmeaReader*  r,
                            Token        bearing )
{
  double  alt;
  Token   tok = bearing;

  if (tok.p >= tok.end)
    return -1;

  r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
  r->fix.bearing  = str2float(tok.p, tok.end);
  return 0;
}


static int
nmea_reader_update_speed( NmeaReader*  r,
                          Token        speed )
{
  double  alt;
  Token   tok = speed;

  if (tok.p >= tok.end)
    return -1;

  r->fix.flags   |= GPS_LOCATION_HAS_SPEED;
  r->fix.speed    = str2float(tok.p, tok.end) / 1.85;
  return 0;
}

static int
add_prn_plus(int prn, SV_TYPE sv_type) {    // add prn plus
  if (sv_type == BDS_SV && prn < PRN_PLUS_BDS)
    prn += PRN_PLUS_BDS;
  else if(sv_type == GLONASS_SV && prn < PRN_PLUS_GLN)
    prn += PRN_PLUS_GLN;
  return prn;
}


static void
nmea_reader_encode_sv_status(NmeaReader*  r) {    // encode used_in_fix flag
  int i;

  // if num_svs is larger than GPS_MAX_SVS, set num_svs to GPS_MAX_SVS
  if ( r->sv_status.num_svs > GPS_MAX_SVS)
    r->sv_status.num_svs = GPS_MAX_SVS;      // this will prevent overflow crash
  for ( i = 0; i < r->sv_status.num_svs; ++i)
  {
    GpsSvInfo *info = &(r->sv_status.sv_list[i]);
    int prn = info->prn;
    info->azimuth = (int)info->azimuth;
    if (r->sv_used_in_fix[prn]) {
      info->azimuth += 720;
    }
  }
}

static void
nmea_reader_parse( NmeaReader*  r )
{
  /* we received a complete sentence, now parse it to generate
   * a new GPS fix...
   */
  NmeaTokenizer  tzer[1];
  Token          tok;
  int         sv_type;

#if NMEA_DEBUG
  D("Received: '%.*s'", r->pos, r->in);
#endif
  if (r->pos < 9) {
    D("Too short. discarded.");
    return;
  }

  nmea_tokenizer_init(tzer, r->in, r->in + r->pos);
#if NMEA_DEBUG
  {
    int  n;
    D("Found %d tokens", tzer->count);
    for (n = 0; n < tzer->count; n++) {
      Token  tok = nmea_tokenizer_get(tzer,n);
      D("%2d: '%.*s'", n, tok.end-tok.p, tok.p);
    }
  }
#endif

  tok = nmea_tokenizer_get(tzer, 0);
  if (tok.p + 5 > tok.end) {
#if NMEA_DEBUG
    D("sentence id '%.*s' too short, ignored.", tok.end-tok.p, tok.p);
#endif
    return;
  }

  if (memcmp(tok.p, "BD", 2) == 0) {
    sv_type = BDS_SV;
#if NMEA_DEBUG
    D("BDS satellites");
#endif
  }
  else if (memcmp(tok.p, "GL", 2) == 0 ) {
    sv_type = GLONASS_SV;
#if NMEA_DEBUG
    D("GLONASS satellites");
#endif
  }
  else {
    sv_type = GPS_SV;
#if NMEA_DEBUG
    D("GPS satellites");
#endif
  }
  // ignore first two characters.
  tok.p += 2;
  if ( !memcmp(tok.p, "GGA", 3) ) {
    // GPS fix
    Token  tok_time          = nmea_tokenizer_get(tzer,1);
    Token  tok_latitude      = nmea_tokenizer_get(tzer,2);
    Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,3);
    Token  tok_longitude     = nmea_tokenizer_get(tzer,4);
    Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,5);
    Token  tok_isPix         = nmea_tokenizer_get(tzer,6);
    Token  tok_altitude      = nmea_tokenizer_get(tzer,9);
    Token  tok_altitudeUnits = nmea_tokenizer_get(tzer,10);

    if (tok_isPix.p[0] == '1') {
      nmea_reader_update_time(r, tok_time);
      nmea_reader_update_latlong(r, tok_latitude,
                                 tok_latitudeHemi.p[0],
                                 tok_longitude,
                                 tok_longitudeHemi.p[0]);
      nmea_reader_update_altitude(r, tok_altitude, tok_altitudeUnits);
    }
    memset(r->sv_used_in_fix, 0, MAX_SV_PRN);
  } else if ( !memcmp(tok.p, "GSA", 3) ) {
#if GPS_SV_INCLUDE

    Token  tok_fixStatus   = nmea_tokenizer_get(tzer, 2);
    int i;

    if (tok_fixStatus.p[0] != '\0' && tok_fixStatus.p[0] != '1') {

      Token  tok_accuracy      = nmea_tokenizer_get(tzer, 15);

      nmea_reader_update_accuracy(r, tok_accuracy);   // pdop

      for (i = 3; i <= 14; ++i) {

        Token  tok_prn  = nmea_tokenizer_get(tzer, i);
        int prn = add_prn_plus(str2int(tok_prn.p, tok_prn.end), sv_type);
        if (prn > 0 && prn < MAX_SV_PRN)
          r->sv_used_in_fix[prn] = 1;
      }

    }
#endif
    // do something ?
  } else if ( !memcmp(tok.p, "RMC", 3) ) {
    Token  tok_time          = nmea_tokenizer_get(tzer,1);
    Token  tok_fixStatus     = nmea_tokenizer_get(tzer,2);
    Token  tok_latitude      = nmea_tokenizer_get(tzer,3);
    Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,4);
    Token  tok_longitude     = nmea_tokenizer_get(tzer,5);
    Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,6);
    Token  tok_speed         = nmea_tokenizer_get(tzer,7);
    Token  tok_bearing       = nmea_tokenizer_get(tzer,8);
    Token  tok_date          = nmea_tokenizer_get(tzer,9);

#if NMEA_DEBUG
    D("in RMC, fixStatus=%c", tok_fixStatus.p[0]);
#endif
    if (tok_fixStatus.p[0] == 'A')
    {
      nmea_reader_update_date( r, tok_date, tok_time );

      nmea_reader_update_latlong( r, tok_latitude,
                                  tok_latitudeHemi.p[0],
                                  tok_longitude,
                                  tok_longitudeHemi.p[0] );

      nmea_reader_update_bearing( r, tok_bearing );
      nmea_reader_update_speed  ( r, tok_speed );
    }
#if GPS_SV_INCLUDE
    r->sv_status_changed = 1;   // update sv status when receive gps, that's last sv status.
#endif

  } else if ( !memcmp(tok.p, "GSV", 3) ) {
#if GPS_SV_INCLUDE
    Token  tok_noSatellites  = nmea_tokenizer_get(tzer, 3);
    int    noSatellites = str2int(tok_noSatellites.p, tok_noSatellites.end);

    if (noSatellites > 0) {

      Token  tok_noSentences   = nmea_tokenizer_get(tzer, 1);
      Token  tok_sentence      = nmea_tokenizer_get(tzer, 2);

      int sentence = str2int(tok_sentence.p, tok_sentence.end);
      int totalSentences = str2int(tok_noSentences.p, tok_noSentences.end);
      int curr;
      int i;


      if (sentence == 1) {
        r->sv_num = 0;
      }

      curr = r->sv_status.num_svs;

      i = 0;
      // max 4 group sv info in one sentence
      while (i < 4 && r->sv_num < noSatellites) {

        Token  tok_prn = nmea_tokenizer_get(tzer, i * 4 + 4);
        Token  tok_elevation = nmea_tokenizer_get(tzer, i * 4 + 5);
        Token  tok_azimuth = nmea_tokenizer_get(tzer, i * 4 + 6);
        Token  tok_snr = nmea_tokenizer_get(tzer, i * 4 + 7);

        if (curr >= 0 && curr < GPS_MAX_SVS) {  // prevent from overflow
          r->sv_status.sv_list[curr].prn = add_prn_plus(str2int(tok_prn.p, tok_prn.end), sv_type);
          r->sv_status.sv_list[curr].elevation = str2float(tok_elevation.p, tok_elevation.end);
          r->sv_status.sv_list[curr].azimuth = str2float(tok_azimuth.p, tok_azimuth.end);
          r->sv_status.sv_list[curr].snr = str2float(tok_snr.p, tok_snr.end);
        }
        r->sv_status.num_svs += 1;
        r->sv_num += 1;

        curr += 1;

        i += 1;
      }
      /*
         if (sentence == totalSentences) {
      //r->sv_status_changed = 1;
      }
       */
#if NMEA_DEBUG
      D("GSV message with total satellites %d", noSatellites);
#endif

    }
    //       else if (noSatellites == 0) {

    //   D("%s: GSV message, There are no satellites %d", __FUNCTION__, noSatellites);
    //   //[SMIT] wwwen: While we can't get Satellites, we report a simular Satellites(10) to test UART
    //   //r->sv_status_changed = 1;
    //   r->sv_status.num_svs = 0;
    //   r->sv_status.sv_list[0].prn = 10;
    //   r->sv_status.sv_list[0].elevation = 0;
    //   r->sv_status.sv_list[0].azimuth = 0;
    //   r->sv_status.sv_list[0].snr = -1;
    //   r->sv_status.num_svs += 1;
    // }

#endif
  } else {
    tok.p -= 2;
#if NMEA_DEBUG
    D("unknown sentence '%.*s", tok.end-tok.p, tok.p);
#endif
  }
  if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG) {
    r->fix.flags |=GPS_LOCATION_HAS_SPEED;
    r->fix.flags |=GPS_LOCATION_HAS_ACCURACY;
    r->fix.flags |=GPS_LOCATION_HAS_BEARING;
    r->fix.flags |=GPS_LOCATION_HAS_ALTITUDE;
#if NMEA_DEBUG
    char   temp[256];
    char*  p   = temp;
    char*  end = p + sizeof(temp);
    time_t time;
    struct tm   utc;

    p += snprintf( p, end-p, "sending fix" );
    if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG) {
      p += snprintf(p, end-p, " lat=%g lon=%g", r->fix.latitude, r->fix.longitude);
    }
    if (r->fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
      p += snprintf(p, end-p, " altitude=%g", r->fix.altitude);
    }
    if (r->fix.flags & GPS_LOCATION_HAS_SPEED) {
      p += snprintf(p, end-p, " speed=%g", r->fix.speed);
    }
    if (r->fix.flags & GPS_LOCATION_HAS_BEARING) {
      p += snprintf(p, end-p, " bearing=%g", r->fix.bearing);
    }
    if (r->fix.flags & GPS_LOCATION_HAS_ACCURACY) {
      p += snprintf(p,end-p, " accuracy=%g", r->fix.accuracy);
    }

    time = r->fix.timestamp / 1000;
    gmtime_r( &time, &utc );
    p += snprintf(p, end-p, " time=%s", asctime( &utc ));
    D("%s", temp);
#endif
    if (r->callback) {
      r->callback( &r->fix );
      r->fix.flags = 0;
    }
    else {
#if NMEA_DEBUG
      D("no callback, keeping data until needed !");
#endif
    }
    // if (r->status_callback) {
    //     r->status.status = GPS_STATUS_ENGINE_OFF;
    //     r->status_callback(&r->status);
    // }
  }
#if GPS_SV_INCLUDE
  if ( r->sv_status_changed == 1 ) {
    // D("Reprot sv status 1.");
    r->sv_status_changed = 0;
    if (r->sv_callback) {
      // D("Reprot sv status 2.");
      nmea_reader_encode_sv_status(r);
      r->sv_callback(&r->sv_status);

      r->sv_status.num_svs = 0;
      memset(r->sv_used_in_fix, 0, MAX_SV_PRN);
    }
    else {
#if NMEA_DEBUG
      D("no sv callback, keeping data until needed !");
#endif
    }
  }
#endif
}
/*
   static GpsUtcTime get_system_timestamp() {
   struct timeval tp;
   gettimeofday(&tp, NULL);    // get current time
   GpsUtcTime t = tp.tv_sec * 1000 + tp.tv_usec / 1000;
   return t;
   }
 */
static void
nmea_reader_addc( NmeaReader*  r, int  c )
{
  if (r->overflow) {
    r->overflow = (c != '\n');
    return;
  }

  if (r->pos >= (int) sizeof(r->in)-1 ) {
    r->overflow = 1;
    r->pos      = 0;
    return;
  }

  r->in[r->pos] = (char)c;
  r->pos       += 1;

  if (c == '\n') {
    nmea_reader_parse( r );
    if (r->nmea_callback) {
      r->nmea_callback( r->fix.timestamp, r->in, r->pos );
    }
    else {
#if NMEA_DEBUG
      D("No nmea callback");
#endif
    }
    r->pos = 0;
  }
}


/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       C O N N E C T I O N   S T A T E                 *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

/* commands sent to the gps thread */
enum {
  CMD_QUIT  = 0,
  CMD_START = 1,
  CMD_STOP  = 2
};


/* this is the state of our connection to the qemu_gpsd daemon */
typedef struct {
  int                     init;
  int                     fd;
  GpsCallbacks            callbacks;
  pthread_t               thread;
  int                     control[2];
  char                    device[32];
  int                     speed;
} GpsState;

static GpsState  _gps_state[1];

#ifdef SUPL_ENABLED
static time_t 
utc_time(int week, long tow) {
  time_t t;

  /* Jan 5/6 midnight 1980 - beginning of GPS time as Unix time */
  t = 315964801;

  /* soon week will wrap again, uh oh... */
  /* TS 44.031: GPSTOW, range 0-604799.92, resolution 0.08 sec, 23-bit presentation */
  t += (1024 + week) * 604800 + tow*0.08;

  return t;
}

static int 
supl_consume_1(supl_assist_t *ctx) {
  if (ctx->set & SUPL_RRLP_ASSIST_REFLOC) {
    D("Reference Location:\n");
    D("  Lat: %f\n", ctx->pos.lat);
    D("  Lon: %f\n", ctx->pos.lon);
    D("  Uncertainty: %d (%.1f m)\n",
      ctx->pos.uncertainty, 10.0*(pow(1.1, ctx->pos.uncertainty)-1));
  }

  if (ctx->set & SUPL_RRLP_ASSIST_REFTIME) {
    time_t t;

    t = utc_time(ctx->time.gps_week, ctx->time.gps_tow);

    D("Reference Time:\n");
    D("  GPS Week: %ld\n", ctx->time.gps_week);
    D("  GPS TOW:  %ld %lf\n", ctx->time.gps_tow, ctx->time.gps_tow*0.08);
    D("  ~ UTC:    %s", ctime(&t));
  }

  if (ctx->set & SUPL_RRLP_ASSIST_IONO) {
    D("Ionospheric Model:\n");
    D("  # a0 a1 a2 b0 b1 b2 b3\n");
    D("  %g, %g, %g",
      ctx->iono.a0 * pow(2.0, -30),
      ctx->iono.a1 * pow(2.0, -27),
      ctx->iono.a2 * pow(2.0, -24));
    D(" %g, %g, %g, %g\n",
      ctx->iono.b0 * pow(2.0, 11),
      ctx->iono.b1 * pow(2.0, 14),
      ctx->iono.b2 * pow(2.0, 16),
      ctx->iono.b3 * pow(2.0, 16));
  }

  if (ctx->set & SUPL_RRLP_ASSIST_UTC) {
    D("UTC Model:\n");
    D("  # a0, a1 delta_tls tot dn\n");
    D("  %g %g %d %d %d %d %d %d\n",
      ctx->utc.a0 * pow(2.0, -30),
      ctx->utc.a1 * pow(2.0, -50),
      ctx->utc.delta_tls,
      ctx->utc.tot, ctx->utc.wnt, ctx->utc.wnlsf,
      ctx->utc.dn, ctx->utc.delta_tlsf);
  }

  if (ctx->cnt_eph) {
    int i;

    D("Ephemeris:");
    D(" %d satellites\n", ctx->cnt_eph);
    D("  # prn delta_n M0 A_sqrt OMEGA_0 i0 w OMEGA_dot i_dot Cuc Cus Crc Crs Cic Cis");
    D(" toe IODC toc AF0 AF1 AF2 bits ura health tgd OADA\n");

    for (i = 0; i < ctx->cnt_eph; i++) {
      struct supl_ephemeris_s *e = &ctx->eph[i];

      D("  %d %g %g %g %g %g %g %g %g",
        e->prn,
        e->delta_n * pow(2.0, -43),
        e->M0 * pow(2.0, -31),
        e->A_sqrt * pow(2.0, -19),
        e->OMEGA_0 * pow(2.0, -31),
        e->i0 * pow(2.0, -31),
        e->w * pow(2.0, -31),
        e->OMEGA_dot * pow(2.0, -43),
        e->i_dot * pow(2.0, -43));
      D(" %g %g %g %g %g %g",
        e->Cuc * pow(2.0, -29),
        e->Cus * pow(2.0, -29),
        e->Crc * pow(2.0, -5),
        e->Crs * pow(2.0, -5),
        e->Cic * pow(2.0, -29),
        e->Cis * pow(2.0, -29));
      D(" %g %u %g %g %g %g",
        e->toe * pow(2.0, 4),
        e->IODC,
        e->toc * pow(2.0, 4),
        e->AF0 * pow(2.0, -31),
        e->AF1 * pow(2.0, -43),
        e->AF2 * pow(2.0, -55));
      D(" %d %d %d %d %d\n",
        e->bits,
        e->ura,
        e->health,
        e->tgd,
        e->AODA * 900);
    }
  }

  if (ctx->cnt_alm) {
    int i;

    D("Almanac:");
    D(" %d satellites\n", ctx->cnt_alm);
    D("  # prn e toa Ksii OMEGA_dot A_sqrt OMEGA_0 w M0 AF0 AF1\n");

    for (i = 0; i < ctx->cnt_alm; i++) {
      struct supl_almanac_s *a = &ctx->alm[i];

      D("  %d %g %g %g %g ",
        a->prn,
        a->e * pow(2.0, -21),
        a->toa * pow(2.0, 12),
        a->Ksii * pow(2.0, -19),
        a->OMEGA_dot * pow(2.0, -38));
      D("%g %g %g %g %g %g\n",
        a->A_sqrt * pow(2.0, -11),
        a->OMEGA_0 * pow(2.0, -23),
        a->w * pow(2.0, -23),
        a->M0 * pow(2.0, -23),
        a->AF0 * pow(2.0, -20),
        a->AF1 * pow(2.0, -38));
    }
  }

  return 1;
}

static int 
supl2cas_aid(supl_assist_t *ctx, unsigned char *buff) {
  D("SUPL 2 Casic Aid.");
  GPS_FIX_EPHEMERIS_STR uTempGpsEph;
  AID_INI_STR uTempAidIni;
  FIX_UTC_STR uTempUtc;
  FIX_IONO_STR uTempIon;

  int cnt;
  int length = 0;

  memset(&uTempAidIni, 0, sizeof(AID_INI_STR));
  supl2cas_ini(ctx, &uTempAidIni);
  if (uTempAidIni.flags) {
    length += cas_make_msg(ID_AID_INI,     (int *)(&uTempAidIni),   sizeof(uTempAidIni),      buff + length);
    D("Pack Casic Ini message.");
  }
  for (cnt = 0; cnt < ctx->cnt_eph; cnt++) {
    if (ctx->set & SUPL_RRLP_ASSIST_REFTIME) {
      memset(&uTempGpsEph, 0, sizeof(GPS_FIX_EPHEMERIS_STR));
      supl2cas_eph((unsigned short)ctx->time.gps_week, &ctx->eph[cnt], &uTempGpsEph);
      if (uTempGpsEph.valid != NAVIGATION_MESSAGE_AVAILABLE)
        continue;
      length += cas_make_msg(ID_RXM_GPS_EPH,   (int *)(&uTempGpsEph),  sizeof(GPS_FIX_EPHEMERIS_STR),  buff + length);
      D("Pack Casic Eph message %d.", cnt);
    }
  }

  if (ctx->set & SUPL_RRLP_ASSIST_UTC) {
    memset(&uTempUtc, 0, sizeof(FIX_UTC_STR));
    supl2cas_utc(&ctx->utc, &uTempUtc);
    if (uTempUtc.valid == NAVIGATION_MESSAGE_AVAILABLE) {
      length += cas_make_msg(ID_RXM_GPS_UTC,   (int *)(&uTempUtc),    sizeof(FIX_UTC_STR),      buff + length);
      D("Pack Casic GPS_UTC message.");
    }
  }

  if (ctx->set & SUPL_RRLP_ASSIST_IONO) {
    memset(&uTempIon, 0, sizeof(FIX_IONO_STR));
    supl2cas_iono(&ctx->iono, &uTempIon);
    if (uTempIon.valid == NAVIGATION_MESSAGE_AVAILABLE) {
      length += cas_make_msg(ID_RXM_GPS_ION,   (int *)(&uTempIon),    sizeof(FIX_IONO_STR),      buff + length);
      D("Pack Casic GPS_ION message.");
    }
  }

  return length;
}

static void 
supl_thread(void *arg) {
  GpsState *state = (GpsState *)arg;
  unsigned char buff[4096];
  int len = 0;
  int err = 0;
  int fd = state->fd;
  supl_assist_t assist;

#if SUPL_TEST
  if (fd != -1) {
    char reboot_cmd[] = "$PCAS10,2*1E\r\n";
    write(fd, reboot_cmd, strlen(reboot_cmd));
  }
#endif

  supl_ctx_new(&supl_ctx);
  agpsRilCallbacks->request_refloc(AGPS_RIL_REQUEST_REFLOC_CELLID);
  agpsRilCallbacks->request_setid(AGPS_RIL_REQUEST_SETID_MSISDN);
  usleep(1000 * 1000);
  
  if (supl_ctx.p.set == 0) {
    D("No cell info present.");
#if SUPL_TEST
    supl_set_lte_cell(&supl_ctx, 460, 0, 22548, 193790209, 0);
#else
    return;
#endif
  }

  if (supl_ctx.p.msisdn[0] == 0) {
    D("No msisdn present.");
    supl_set_msisdn(&supl_ctx, "+8613588889999");
  }

  err = supl_get_assist(&supl_ctx, supl_host, supl_port, &assist);
  if (err < 0) {
    D("SUPL protocol error %d\n", err);
    return;
  }
#if SUPL_TEST
  supl_consume_1(&assist);
#endif

  len = supl2cas_aid(&assist, buff);
  if (len > 0 && fd != -1) {
    write(fd, buff, len);
    D("Send CasicAidMessage: %d bytes.", len); 
  }
  last_supl_time = time(NULL);
  D("Update last supl time: %lu", last_supl_time);
#if SUPL_TEST
  FILE *f = fopen("/data/agpshal.bin", "wb");
  if (f != NULL) {
    fwrite(buff, 1, len, f);
    fclose(f);
  }
#endif

  supl_ctx_free(&supl_ctx);
}

static int
is_supl_needed() {
  time_t now = time(NULL);  
  if (now - last_supl_time > 3600) {
    D("Supl needed: %lu vs %lu", now, last_supl_time);
    return 1;
  }
#if SUPL_TEST
  return 1;
#else
  return 0;
#endif
}

/*
static void 
supl_start() {
  GpsState *state = _gps_state;
  int thread = state->callbacks.create_thread_cb("supl_thread", supl_thread, state);
  if (!thread) {
    D("Could not create supl thread: %s", strerror(errno));
    return;
  }
}
*/
#endif


#if GPS_SV_INCLUDE
static char * gps_idle_on   = "$PCGDC,IDLEON,1,*1\r\n";
static char * gps_idle_off  = "$PCGDC,IDLEOFF,1,*1\r\n";
#endif

static void
gps_state_done( GpsState*  s )
{
  // tell the thread to quit, and wait for it
  char   cmd = CMD_QUIT;
  void*  dummy;
  write( s->control[0], &cmd, 1 );
  pthread_join(s->thread, &dummy);

  // close the control socket pair
  close( s->control[0] );
  s->control[0] = -1;
  close( s->control[1] );
  s->control[1] = -1;

  // close connection to the QEMU GPS daemon
  close( s->fd );
  s->fd = -1;
  s->init = 0;
}

static void
gps_state_start( GpsState*  s )
{
  char  cmd = CMD_START;
  int   ret;

  do {
    ret=write( s->control[0], &cmd, 1 );
  }
  while (ret < 0 && errno == EINTR);

  if (ret != 1)
    D("Could not send CMD_START command: ret=%d: %s",
      ret, strerror(errno));

#if GPS_SV_INCLUDE
  write(s->fd,gps_idle_off,strlen(gps_idle_off));
  D("%s",gps_idle_off);
#endif

#ifdef SUPL_ENABLED
  if (is_supl_needed()) {
    int thread = s->callbacks.create_thread_cb("supl_thread", supl_thread, s);
    if (!thread) {
      D("Could not create supl thread: %s", strerror(errno));
      return;
    }
  }
#endif
}


static void
gps_state_stop( GpsState*  s )
{
  char  cmd = CMD_STOP;
  int   ret;

  do {
    ret=write( s->control[0], &cmd, 1 );
  }
  while (ret < 0 && errno == EINTR);

  if (ret != 1)
    D("Could not send CMD_STOP command: ret=%d: %s",
      ret, strerror(errno));

#if GPS_SV_INCLUDE
  write(s->fd,gps_idle_on,strlen(gps_idle_on));
  D("%s",gps_idle_on);
#endif
}


static int
epoll_register( int  epoll_fd, int  fd )
{
  struct epoll_event  ev;
  int                 ret, flags;

  /* important: make the fd non-blocking */
  flags = fcntl(fd, F_GETFL);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  ev.events  = EPOLLIN;
  ev.data.fd = fd;
  do {
    ret = epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
  } while (ret < 0 && errno == EINTR);
  return ret;
}


static int
epoll_deregister( int  epoll_fd, int  fd )
{
  int  ret;
  do {
    ret = epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, NULL );
  } while (ret < 0 && errno == EINTR);
  return ret;
}

/* this is the main thread, it waits for commands from gps_state_start/stop and,
 * when started, messages from the QEMU GPS daemon. these are simple NMEA sentences
 * that must be parsed to be converted into GPS fixes sent to the framework
 */
static void
gps_state_thread( void*  arg )
{
  GpsState*   state = (GpsState*) arg;
  NmeaReader  reader[1];
  int         epoll_fd   = epoll_create(2);
  int         started    = 0;
  int         gps_fd     = state->fd;
  int         control_fd = state->control[1];
  int         t_sec = -1;

  nmea_reader_init( reader );

  // register control file descriptors for polling
  epoll_register( epoll_fd, control_fd );
  epoll_register( epoll_fd, gps_fd );

  D("gps thread running");

  // now loop
  for (;;) {
    struct epoll_event   events[2];
    int                  ne, nevents;

    nevents = epoll_wait( epoll_fd, events, 2, -1 );
    if (nevents < 0) {
      if (errno != EINTR)
        D("epoll_wait() unexpected error: %s", strerror(errno));
      continue;
    }
#if NMEA_DEBUG
    D("gps thread received %d events", nevents);
#endif
    for (ne = 0; ne < nevents; ne++) {
      if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
        D("EPOLLERR or EPOLLHUP after epoll_wait() !?");
        return;
      }
      if ((events[ne].events & EPOLLIN) != 0) {
        int  fd = events[ne].data.fd;

        if (fd == control_fd)
        {
          char  cmd = 255;
          int   ret;
          D("gps control fd event");
          do {
            ret = read( fd, &cmd, 1 );
          } while (ret < 0 && errno == EINTR);

          if (cmd == CMD_QUIT) {
            D("gps thread quitting on demand");
            return;
          }
          else if (cmd == CMD_START) {
            if (!started) {
              // version query
              // char cmdbuf[] = "$PCAS06,0*1B\r\n";
              // int ret = write(gps_fd, cmdbuf, sizeof(cmdbuf));
              // D("send GPTXT version query cmd, ret = %d , cmd = %s ", ret, cmdbuf);
              D("gps thread starting  location_cb=%p", state->callbacks.location_cb);
              started = 1;
              nmea_reader_set_nmea_callback( reader, state->callbacks.nmea_cb );
              nmea_reader_set_callback( reader, state->callbacks.location_cb );
              nmea_reader_set_status_callback(reader, state->callbacks.status_cb);
#if GPS_SV_INCLUDE
              nmea_reader_set_sv_callback( reader, state->callbacks.sv_status_cb );
#endif
              if (reader->status_callback) {
                reader->status.status = GPS_STATUS_SESSION_BEGIN;
                reader->status_callback(&reader->status);
              }

            }
          }
          else if (cmd == CMD_STOP) {
            if (started) {
              D("gps thread stopping");
              started = 0;
              if (reader->status_callback) {
                reader->status.status = GPS_STATUS_SESSION_END;
                reader->status_callback(&reader->status);
              }
              nmea_reader_set_nmea_callback( reader, NULL );
              nmea_reader_set_callback( reader, NULL );
              nmea_reader_set_status_callback( reader, NULL );
#if GPS_SV_INCLUDE
              nmea_reader_set_sv_callback( reader, NULL );
#endif

            }
          }
        }
        else if (fd == gps_fd)
        {
          char  buff[128];
          // D("gps fd event");
          for (;;) {
            int  nn, ret;

            ret = read( fd, buff, sizeof( buff ) );
            if (ret < 0) {
              if (errno == EINTR)
                continue;
              if (errno != EWOULDBLOCK)
                D("error while reading from gps daemon socket: %s:", strerror(errno));
              break;
            }

#if NMEA_DEBUG
            D("gps fd received: %.*s bytes: %d", ret, buff, ret);
#endif
            for (nn = 0; nn < ret; nn++)
              nmea_reader_addc( reader, buff[nn] );
          }
          // D("gps fd event end");
        }
        else
        {
          D("epoll_wait() returned unkown fd %d ?", fd);
        }
      }
    }
  }

}

// open tty
static void
gps_state_init( GpsState*  state)
{
  struct termios termios;

  state->init       = 1;
  state->control[0] = -1;
  state->control[1] = -1;
  state->fd         = -1;


  strcpy(state->device, tty_name);
  state->speed = tty_baud;

  state->fd = open(state->device, O_RDWR | O_NONBLOCK | O_NOCTTY);

  if (state->fd < 0) {
    D("Can not open gps tty: %s, errno = %d", state->device, errno);
    return;
  }
  D("gps uart open %s success!", state->device);

  struct termios cfg;
  tcgetattr(state->fd, &cfg);
  cfmakeraw(&cfg);
  cfsetispeed(&cfg, state->speed);
  cfsetospeed(&cfg, state->speed);
  tcsetattr(state->fd, TCSANOW, &cfg);

  D("gps will read from %s", state->device);

  if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, state->control ) < 0 ) {
    D("could not create thread control socket pair: %s", strerror(errno));
    goto Fail;
  }

  state->thread = state->callbacks.create_thread_cb( "gps_state_thread", gps_state_thread, state );

  if ( !state->thread ) {
    D("could not create gps thread: %s", strerror(errno));
    goto Fail;
  }

  // state->callbacks = *callbacks;

  D("gps state initialized");
  return;

Fail:
  gps_state_done( state );
}


/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       I N T E R F A C E                               *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/


static int
zkw_gps_init(GpsCallbacks* callbacks)
{
  GpsState*  s = _gps_state;
  s->callbacks = *callbacks;
  load_conf();
  if (!s->init)
    gps_state_init(s);

  if (s->fd < 0)
    return -1;

  return 0;
}

static void
zkw_gps_cleanup(void)
{
  GpsState*  s = _gps_state;

  if (s->init)
    gps_state_done(s);
}

static void gps_hardware_power( int state )
{
  int fd = open( "/proc/gps", O_RDWR );
  if ( fd <= 0 ) {
    D( "/proc/gps open faild, errno %d\r\n", errno );
    return;
  }
  if ( state ) {
    write( fd , "1", 1 );
  } else {
    write( fd , "0", 1 );
  }
  close( fd );
}

static int
zkw_gps_start()
{
  GpsState*  s = _gps_state;

  if (!s->init) {
    D("%s: called with uninitialized state !!", __FUNCTION__);
    return -1;
  }
  gps_hardware_power( 1 );
  D("%s: called", __FUNCTION__);
  gps_state_start(s);
  return 0;
}


static int
zkw_gps_stop()
{
  GpsState*  s = _gps_state;

  if (!s->init) {
    D("%s: called with uninitialized state !!", __FUNCTION__);
    return -1;
  }
  gps_hardware_power( 0 );
  D("%s: called", __FUNCTION__);
  gps_state_stop(s);
  return 0;
}

static int
zkw_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
  return 0;
}

static int
zkw_gps_inject_location(double latitude, double longitude, float accuracy)
{
  return 0;
}

static void
zkw_gps_delete_aiding_data(GpsAidingData flags)
{
  return;
}


static int
zkw_gps_set_position_mode(GpsPositionMode mode, GpsPositionRecurrence recurrence,
                          uint32_t min_interval, uint32_t preferred_accuracy, uint32_t preferred_time)
{
  return 0;
}

static const void*
zkw_gps_get_extension(const char* name)
{
  // no extensions supported
  /*if ( strcmp(name, AGPS_INTERFACE) == 0 ) {
    return &zkwAGpsInterface;
    }
    else*/
  if ( strcmp(name, AGPS_RIL_INTERFACE) == 0 ) {
    return &zkwAGpsRilInterface;
  }
  return NULL;
}



static const GpsInterface  zkwGpsInterface = {
  .size  = sizeof(GpsInterface),
  .init  = zkw_gps_init,
  .start = zkw_gps_start,
  .stop  = zkw_gps_stop,
  .cleanup = zkw_gps_cleanup,
  .inject_time = zkw_gps_inject_time,
  .inject_location = zkw_gps_inject_location,
  .delete_aiding_data = zkw_gps_delete_aiding_data,
  .set_position_mode = zkw_gps_set_position_mode,
  .get_extension = zkw_gps_get_extension,
};

static const GpsInterface* get_gps_interface()
{
  return &zkwGpsInterface;
}

static int open_gps(const struct hw_module_t* module, char const* name,
                    struct hw_device_t** device)
{

  struct gps_device_t *dev = malloc(sizeof(struct gps_device_t));
  memset(dev, 0, sizeof(*dev));

  D("Zkw hal driver, version=%d.%d", module->version_major, module->version_minor);
  dev->common.tag = HARDWARE_DEVICE_TAG;
  dev->common.version = 0;
  dev->common.module = (struct hw_module_t*)module;
  dev->get_gps_interface = get_gps_interface;
  _gps_state->init = 0;

  D("Zkw hal driver opened.");
  *device = (struct hw_device_t*)dev;
  return 0;
}


static struct hw_module_methods_t gps_module_methods = {
  .open = open_gps
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
  .tag = HARDWARE_MODULE_TAG,
  .version_major = 3,
  .version_minor = 21,
  .id            = GPS_HARDWARE_MODULE_ID,
  .name          = "HZZKW GNSS Module",
  .author        = "Jarod Lee",
  .methods       = &gps_module_methods,
};
