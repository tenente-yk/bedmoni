/*! \file tprn.c
    \brief Thermoprinter interface functions
 */

#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include "bedmoni.h"
#include "dview.h"
#include "ep.h"
#include "t1t2.h"
#include "ecgm.h"
#include "cframe.h"
#include "mframe.h"
#include "stconf.h"
#include "grph.h"
#include "port.h"
#include "fifo.h"
#include "demo.h"
#include "utils.h"
#include "crbpio.h"
#include "powerman.h"
#include "tprn.h"
#include "pat.h"

#ifdef UNIX
pthread_mutex_t mutex_tprndata = PTHREAD_MUTEX_INITIALIZER;
#endif

static tprn_cfg_t m_tprn_cfg;

//float tprn_amp_ecg[MAXNUM_TPRN_AMP] = {2.5f, 5.0f, 10.0f, 20.0f, 40.0f};
float tprn_amp[MAXNUM_TPRN_AMP] = {1.0f, 2.0f, 4.0f, 8.0f, 16.0f};
float tprn_amp_mul[NUM_VIEWS] =
{
  2.5f,   // ECG1
  2.5f,   // ECG2
  2.5f,   // ECG3
  2.5f,   // ECG4
  2.5f,   // ECG5
  2.5f,   // ECG6
  2.5f,   // ECG7
  1.0f,
  0.05f,  // SpO2
  1.0f,
  40.0f,  // RESP
  1.0f,   // T1T2
  100.0f, // CO2 /// 1000 * 10mm / 100
};
float tprn_spd[MAXNUM_TPRN_SPD] = {12.5f, 25.0f, 50.0f};

static int tprn_grph_fd;                 // curves
static int tprn_grph_fd_p;               // params
static int fd = 0;                       // fd to tty port

unsigned char tprn_data_buf[TPRN_MAX_FIFO_SIZE];
static fifo_t fifo_tprn_data;
static unsigned long num_lines_to_send = 0;
static const int packet_size = TPRN_PACKET_SIZE;

static unsigned char tprnin_buf[TPRNIN_BUF_SIZE];
int tprnin_ptr = 0;

static unsigned char tprn_hw_status = 0;
static int tprn_status = 0;
static int tprn_avail_size = 0;
static int sent_to_tprn = 0; // how many bytes have been sent to printer

static int tprn_v_old[TPRN_NUM_CURVES];
static double data_ptr[TPRN_NUM_CURVES];
static int data_ptr_r;
static int data_max_res;

static unsigned char cmline3[10*TPRN_PAPER_DPM];
static unsigned char cmline2[10*TPRN_PAPER_DPM];
static unsigned char cmline1[10*TPRN_PAPER_DPM];
static unsigned char cmline0[10*TPRN_PAPER_DPM];

static tprn_stored_data_t tprn_stored_data[TPRN_NUM_CURVES];

static void tprn_feed(void);

static volatile int printing = 0;

void tprn_init(void)
{
  tprn_cfg_t tcfg;
  grph_screeninfo_t gsi;
  int i, j;
  PGDC pgdc;

  tprn_ini_def();

  if ( stconf_read(STCONF_TPRN, &tcfg, sizeof(tprn_cfg_t)) > 0 )
  {
    tprn_cfg_set(&tcfg);
  }

  tprn_grph_fd_p = 0;
  tprn_grph_fd   = 0;

  memset(&gsi, 0, sizeof(grph_screeninfo_t));
  gsi.xres = 1000; // value has to be assigned with TPRN_MAX_FIFO_SIZE
  gsi.yres = TPRN_PAPER_DW;
  gsi.bpp = 1;
  tprn_grph_fd_p = grph_create(&gsi, 0);

  memset(&gsi, 0, sizeof(grph_screeninfo_t));
  gsi.xres = TPRN_MAX_SEC_TO_PRINT*50*TPRN_PAPER_DPM; // 50 - 50 mm/s
  gsi.yres = TPRN_PAPER_DW;
  gsi.bpp = 8;
  tprn_grph_fd = grph_create(&gsi, 0);
  grph_setfgcolor(tprn_grph_fd, 0xffffff);

  fifo_init(&fifo_tprn_data, (unsigned char*)tprn_data_buf, sizeof(tprn_data_buf));

  for (i=0; i<TPRN_NUM_CURVES; i++)
  {
    data_ptr[i] = 0.0f;
    tprn_v_old[i] = 0;
  }
  data_ptr_r = 0;
  data_max_res = (50 * TPRN_MAX_SEC_TO_PRINT * TPRN_PAPER_DPM);

  memset(cmline3, 0, 10*TPRN_PAPER_DPM);
  memset(cmline2, 0, 10*TPRN_PAPER_DPM);
  memset(cmline1, 0, 10*TPRN_PAPER_DPM);
  memset(cmline0, 0, 10*TPRN_PAPER_DPM);
  for (i=0; i<10*TPRN_PAPER_DPM; i++)
  {
    if (i % 4 == 0)
      cmline3[i] = 1;
    if (i % TPRN_PAPER_DPM == 0)
      cmline2[i] = 1;
  }
  cmline1[0] = 1;

  if (tcfg.prnmask.grid)
  {
    unsigned char * line;
    pgdc = (PGDC)tprn_grph_fd;
    for (i=0; i<pgdc->yres; i++)
    {
      if (i % (10*TPRN_PAPER_DPM) == 0)
        line = cmline3;
      else
      if (i % TPRN_PAPER_DPM == 0)
        line = cmline2;
      else
      if (i % 4 == 0)
        line = cmline1;
      else
        line = cmline0;
      for (j=0; j<pgdc->xres; j+=10*TPRN_PAPER_DPM)
        memcpy((unsigned char*)pgdc->addr+i*pgdc->xres+j, line, 10*TPRN_PAPER_DPM);
    }
  }

  memset(&tprn_stored_data, 0, sizeof(tprn_stored_data));

#ifdef TPRN_DISABLED
  fd = 0;
  debug("tprn disabled!!!\n");
#else
  debug("tprn io port: %s\n", TPRNIO_COMMPORT);
#ifdef ARM
  fd = com_open_2( TPRNIO_COMMPORT, B500000);
#else
  fd = com_open(TPRNIO_COMMPORT, BR115200, 0);
#endif
  alarm_clr(TPRN_NOCONNECT);
  if (fd <= 0)
  {
    error("unable to open tprn comm port %s\n", TPRNIO_COMMPORT);
    alarm_set(TPRN_NOCONNECT);
  }
  else
  {
    tprn_command(TPRN_ID_SETUP, 0x02, 0x00, 0x00, 0x00); // set 12.5 mm/s -> retrieve printer hw status
  }
 // fcntl(fd, F_SETFL, fcntl(fd,F_GETFL) & (~O_NONBLOCK));
#endif
}

void tprn_deinit(void)
{
  tprn_cfg_save();

  grph_destroy(tprn_grph_fd_p);
  grph_destroy(tprn_grph_fd);

  tprn_grph_fd_p = 0;
  tprn_grph_fd   = 0;

  if (fd > 0) com_close(fd);
}

void tprn_ini_def(void)
{
  tprn_cfg_t tcfg;
  memset(&m_tprn_cfg, 0, sizeof(m_tprn_cfg));
  tcfg.n = 2;
  tcfg.c[0] = ECG_I;
  tcfg.c[1] = ECG_II;
  tcfg.amp[0] = tcfg.amp[1] = TPRN_AMP_10;
  tcfg.spd = TPRN_SPD_25;
  tcfg.prnmask.grid = tcfg.prnmask.val = 1;
  tcfg.prnmask.width = TPRN_WIDTH_2;
  tcfg.prnmask.brightness = TPRN_BRIGHTNESS_100;
  tcfg.delay1 = 1;
  tcfg.delay2 = 3;
  tcfg.pos = 20;
  tprn_cfg_set(&tcfg);
}

int tprn_cfg_get(tprn_cfg_t * ptr)
{
  if (!ptr) return -1;

#ifdef UNIX
  pthread_mutex_lock(&mutex_tprndata);
#endif

  memcpy(ptr, &m_tprn_cfg, sizeof(tprn_cfg_t));

#ifdef UNIX
  pthread_mutex_unlock(&mutex_tprndata);
#endif

  return 1;
}

int tprn_cfg_set(tprn_cfg_t * ptr)
{
  if (!ptr) return -1;

#ifdef UNIX
  pthread_mutex_lock(&mutex_tprndata);
#endif

  memcpy(&m_tprn_cfg, ptr, sizeof(tprn_cfg_t));

 // cframe_t * pcframe = cframe_getptr();
 // int i;
 // for (i=0; i<TPRN_NUM_CURVES; i++)
 //   pcframe->tprn_c[i] = m_tprn_cfg.c[i];

#ifdef UNIX
  pthread_mutex_unlock(&mutex_tprndata);
#endif

  return 1;
}

static void tprn_put_line(int x)
{
  PGDC pgdc;
  int i, len;
  unsigned char buf[500];
  unsigned char hw_buf[500];
  unsigned char b;
  unsigned char crc;

  pgdc = (PGDC) tprn_grph_fd;

  for (i=0; i<pgdc->yres; i++)
  {
    if (pgdc->bpp == 8)
      buf[i] = *((unsigned char*)pgdc->addr + i*pgdc->xres + x);
    else
      assert(0);
  }

  len = pgdc->yres;
  crc = 0;

  hw_buf[0] = TPRN_START;     // start
  hw_buf[1] = TPRN_ID_DATA;   // reserved
  hw_buf[2] = len / 8;        // len

  b = 0;
  for (i=0; i<len; i++)
  {
    if (buf[i] != 0x00)
    {
      b |= (1 << (i & 0x7));
#ifdef TPRN_TRANSPARENT_INKS
      b = 0;
#endif
    }
    if ((i & 0x7) == 0x7)
    {
      hw_buf[3+len/8-i/8-1] = b;
      b = 0;
    }
  }

  crc = 0;
  for (i=0; i<packet_size-1; i++)
    crc += hw_buf[i];

  hw_buf[3+len/8] = crc;      // crc

 // packet_size = len/8 + 4;

  fifo_put_bytes(&fifo_tprn_data, hw_buf, packet_size);
}

static void tprn_put_8data(unsigned char * buf, int len, int n)
{
  int i, j;
  unsigned char hw_buf[100];
  unsigned char b;
  unsigned char crc;

  for (j=0; j<n; j++)
  {
    crc = 0;

    hw_buf[0] = TPRN_START;   // start
    hw_buf[1] = TPRN_ID_DATA; // reserved
    hw_buf[2] = len / 8;      // len

    b = 0;
    for (i=0; i<len; i++)
    {
      if ((buf[i]>>j)&0x1)
      {
        b |= (1 << (i & 0x7));
#ifdef TPRN_TRANSPARENT_INKS
        b = 0;
#endif
      }
      if ((i & 0x7) == 0x7)
      {
        hw_buf[3+len/8-i/8-1] = b;
        b = 0;
      }
    }

   // packet_size = len/8 + 4;

    crc = 0;
    for (i=0; i<packet_size-1; i++)
      crc += hw_buf[i];

    hw_buf[3+len/8] = crc;    // crc

#if 0
    assert ( fifo_put_bytes(&fifo_tprn_data, hw_buf, packet_size) == packet_size );
#else
    if (fifo_put_bytes(&fifo_tprn_data, hw_buf, packet_size) != packet_size)
    {
      error("assert skipped: %s line %d\n", __FILE__, __LINE__);
    }
#endif
  }
}

static int tprn_flush_info(void)
{
  PGDC pgdc = (PGDC) tprn_grph_fd_p;
  tprn_cfg_t tprn_cfg;
  pat_pars_t lpp;
  unsigned char a[500];
  char s[200], s2[128];
  int x, y, i, n, y0[TPRN_NUM_CURVES], xw;
  struct tm ltm;
  ecg_data_t ecg_data;
  spo2_data_t spo2_data;
  nibp_data_t nibp_data;
  resp_data_t resp_data;
  t1t2_data_t t1t2_data;
  co2_data_t co2_data;
  int w, h;
  long v;
//  rect_t rc;

  n = 0;
  w = 3*TPRN_PAPER_DPM;

  tprn_cfg_get(&tprn_cfg);
  pat_get(&lpp);

  grph_fillrect(tprn_grph_fd_p, 0, 0, pgdc->xres-1, pgdc->yres-1, 0x000000);

  grph_setfgcolor(tprn_grph_fd_p, 0xffffff);
  grph_setfont(tprn_grph_fd_p, ARIAL_10);
  ids2string(pat_cat_ids[lpp.type], s);
  grph_textout(tprn_grph_fd_p, 20, 20, s, strlen(s));
  sprintf(s, "N %d", lpp.cardno);
  grph_textout(tprn_grph_fd_p, 20, 40, s, strlen(s));
  sprintf(s, "B %d", lpp.bedno);
  grph_textout(tprn_grph_fd_p, 20, 60, s, strlen(s));

  ids2string(IDS_DATE_OF_PRINT, s);
  grph_textout(tprn_grph_fd_p, 20, TPRN_PAPER_DW-60, s, strlen(s));

  read_hwclock_data(&ltm);
  sprintf(s, "%02d:%02d:%02d", ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
  grph_textout(tprn_grph_fd_p, 20, TPRN_PAPER_DW-40, s, strlen(s));
  sprintf(s, "%02d.%02d.%04d", ltm.tm_mday, ltm.tm_mon+1, ltm.tm_year+1900);
  grph_textout(tprn_grph_fd_p, 20, TPRN_PAPER_DW-20, s, strlen(s));

  grph_setfont(tprn_grph_fd_p, ARIAL_12);

  if (demo_mode)
  {
    demo_cfg_t dcfg;
    struct stat st;
    struct tm *ptm;
    demo_cfg_get(&dcfg);
    demo_record_stat(dcfg.fno, &st);
    ptm = gmtime(&st.st_mtime);
    ids2string(IDS_DEMO_LARGE, s);
    grph_textout(tprn_grph_fd_p, 28, 100, s, strlen(s));
    sprintf(s, "%02d.%02d.%04d", ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900);
    grph_textout(tprn_grph_fd_p, 15, 120, s, strlen(s));
    sprintf(s, "%02d:%02d", (int)(ptm->tm_hour+ltm.tm_gmtoff/3600)%24, ptm->tm_min);
    grph_textout(tprn_grph_fd_p, 30, 140, s, strlen(s));
  }

  xw = 120;

  if (tprn_cfg.prnmask.val)
  {
    unit_get_data(ECG, &ecg_data);
    unit_get_data(SPO2, &spo2_data);
    unit_get_data(NIBP, &nibp_data);
    unit_get_data(RESP, &resp_data);
    unit_get_data(T1T2, &t1t2_data);
    unit_get_data(CO2, &co2_data);

    y = 0;

    // ECG
    if (unit_isenabled(ECG))
    {
      ids2string(IDS_HR, s);
      grph_textout(tprn_grph_fd_p, 120, y+20,  s, strlen(s));
      grph_textout(tprn_grph_fd_p, 120, y+40,  "ST I", strlen("ST I"));
      grph_textout(tprn_grph_fd_p, 120, y+60,  "ST II", strlen("ST II"));
      grph_textout(tprn_grph_fd_p, 120, y+80,  "ST III", strlen("ST III"));
      grph_textout(tprn_grph_fd_p, 120, y+100, "ST aVR", strlen("ST aVR"));
      grph_textout(tprn_grph_fd_p, 120, y+120, "ST aVL", strlen("ST aVL"));
      grph_textout(tprn_grph_fd_p, 120, y+140, "ST aVF", strlen("ST aVF"));
      grph_textout(tprn_grph_fd_p, 120, y+160, "ST V", strlen("ST V"));

      if (ecg_data.hr != UNDEF_VALUE)
        sprintf(s, "%d", ecg_data.hr);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+20, s, strlen(s));
      for (i=0; i<NUM_ECG; i++)
      {
        if (ecg_data.st[i] != UNDEF_VALUE)
          sprintf(s, "%d", ecg_data.st[i]);
        else
          sprintf(s, "-");
        grph_textout(tprn_grph_fd_p, 240, y+40+20*i, s, strlen(s));
      }

      ids2string(IDS_HBPM, s);
      grph_textout(tprn_grph_fd_p, 320, y+20,  s, strlen(s));
      ids2string(IDS_MV, s);
      grph_textout(tprn_grph_fd_p, 320, y+40,  s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+60,  s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+80,  s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+100, s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+120, s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+140, s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+160, s, strlen(s));

      y += 160;
    }

    // SpO2
    if (unit_isenabled(SPO2))
    {
      grph_textout(tprn_grph_fd_p, 120, y+20, "SpO2", strlen("SpO2"));
      ids2string(IDS_PULSE, s);
      grph_textout(tprn_grph_fd_p, 120, y+40, s, strlen(s));

      if (spo2_data.spo2 != UNDEF_VALUE)
        sprintf(s, "%d", spo2_data.spo2);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+20, s, strlen(s));

      if (spo2_data.hr != UNDEF_VALUE)
        sprintf(s, "%d", spo2_data.hr);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+40, s, strlen(s));

      grph_textout(tprn_grph_fd_p, 320, y+20, "%", strlen("%"));
      ids2string(IDS_HBPM, s);
      grph_textout(tprn_grph_fd_p, 320, y+40, s, strlen(s));

      y += 40;
    }

    // NiBP
    if (unit_isenabled(NIBP))
    {
      ids2string(IDS_NIBP_SS_BRF, s);
      grph_textout(tprn_grph_fd_p, 120, y+20, s, strlen(s));
      ids2string(IDS_NIBP_DD_BRF, s);
      grph_textout(tprn_grph_fd_p, 120, y+40, s, strlen(s));
      ids2string(IDS_NIBP_CC_BRF, s);
      grph_textout(tprn_grph_fd_p, 120, y+60, s, strlen(s));
      ids2string(IDS_NIBP_MEAS_TIME, s);
      grph_textout(tprn_grph_fd_p, 120, y+80, s, strlen(s));

      if (nibp_data.sd != UNDEF_VALUE)
        sprintf(s, "%d", nibp_data.sd);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+20, s, strlen(s));

      if (nibp_data.dd != UNDEF_VALUE)
        sprintf(s, "%d", nibp_data.dd);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+40, s, strlen(s));

      if (nibp_data.md != UNDEF_VALUE)
        sprintf(s, "%d", nibp_data.md);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+60, s, strlen(s));

      assert(sizeof(v) == sizeof(trts_t));
      memcpy(&v, &nibp_data.trts_meas, sizeof(v));
      if (v != UNDEF_VALUE && nibp_data.trts_meas.year > 110)
        sprintf(s, "%02d:%02d  %02d.%02d.%04d", nibp_data.trts_meas.hour, nibp_data.trts_meas.min, (int)nibp_data.trts_meas.day, (int)nibp_data.trts_meas.mon, (int)nibp_data.trts_meas.year+1900);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+80, s, strlen(s));

      ids2string(IDS_MM_HG, s);
      grph_textout(tprn_grph_fd_p, 320, y+20, s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+40, s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+60, s, strlen(s));
      // skip nibp meas time

      y += 80;
    }

    // Resp
    if (unit_isenabled(RESP))
    {
      ids2string(IDS_BR, s);
      grph_textout(tprn_grph_fd_p, 120, y+20, s, strlen(s));

      if (resp_data.br != UNDEF_VALUE)
        sprintf(s, "%d", resp_data.br);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+20, s, strlen(s));

      ids2string(IDS_BRPM, s);
      grph_textout(tprn_grph_fd_p, 320, y+20, s, strlen(s));

      y += 20;
    }

    // T
    if (unit_isenabled(T1T2))
    {
      grph_textout(tprn_grph_fd_p, 120, y+20, "T1", strlen("T1"));
      grph_textout(tprn_grph_fd_p, 120, y+40, "T2", strlen("T2"));
      ids2string(IDS_DT, s);
      grph_textout(tprn_grph_fd_p, 120, y+60, s, strlen(s));

      if (t1t2_data.t1 != UNDEF_VALUE)
        sprintf(s, "%.01f", (float)t1t2_data.t1/10);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+20, s, strlen(s));

      if (t1t2_data.t2 != UNDEF_VALUE)
        sprintf(s, "%.01f", (float)t1t2_data.t2/10);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+40, s, strlen(s));

      if (t1t2_data.t1 != UNDEF_VALUE && t1t2_data.t2 != UNDEF_VALUE)
        sprintf(s, "%.01f", (float)(abs(t1t2_data.t1-t1t2_data.t2))/10);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+60, s, strlen(s));

      s[0]=176; // gradus
      s[1]='C';
      s[2]=0;   // null-termination
      grph_textout(tprn_grph_fd_p, 320, y+20, s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+40, s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+60, s, strlen(s));

      y += 60;
    }

    // CO2
    if (unit_isenabled(CO2))
    {
      grph_textout(tprn_grph_fd_p, 120, y+20, "etCO2", strlen("etCO2"));
      grph_textout(tprn_grph_fd_p, 120, y+40, "iCO2", strlen("iCO2"));
      ids2string(IDS_BR, s);
      grph_textout(tprn_grph_fd_p, 120, y+60, s, strlen(s));

      if (co2_data.etco2 != UNDEF_VALUE)
        sprintf(s, "%d", (int)co2_data.etco2);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+20, s, strlen(s));

      if (co2_data.ico2 != UNDEF_VALUE)
        sprintf(s, "%d", (int)co2_data.ico2);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+40, s, strlen(s));

      if (co2_data.br != UNDEF_VALUE)
        sprintf(s, "%d", (int)co2_data.br);
      else
        sprintf(s, "-");
      grph_textout(tprn_grph_fd_p, 240, y+60, s, strlen(s));

      ids2string(IDS_MM_HG, s);
      grph_textout(tprn_grph_fd_p, 320, y+20, s, strlen(s));
      grph_textout(tprn_grph_fd_p, 320, y+40, s, strlen(s));
      ids2string(IDS_BRPM, s);
      grph_textout(tprn_grph_fd_p, 320, y+60, s, strlen(s));

      y += 60;
    }

    xw = 440;

//    xw = 640;
  } // if (tprn_cfg.prnmask.val)

  grph_setfont(tprn_grph_fd_p, MONOS_11);

  n = 0;
  for (i=0; i<TPRN_NUM_CURVES; i++)
  {
    if (tprn_cfg.c[i] != TPRN_UNDEF_CURVE) n++;
  }

  n = min(n, tprn_cfg.n);

  if (n > 0)
  {
    ids2string(IDS_MMPS, s2);

    if (tprn_cfg.spd == TPRN_SPD_12_5)
      sprintf(s, "%.1f %s", tprn_spd[tprn_cfg.spd], s2);
    else
      sprintf(s, "%d %s", (int)(tprn_spd[tprn_cfg.spd]), s2);
    grph_textout(tprn_grph_fd_p, xw, TPRN_PAPER_DW-20, s, strlen(s));
  }

  for (i=0; i<n; i++)
  {
    y0[i] = (i+1)*(TPRN_PAPER_DW / (n + 1));
    ids2string(chan_ids[tprn_cfg.c[i]], s);
    grph_textout(tprn_grph_fd_p, xw, y0[i]-10, s, strlen(s));
    if (chno_is_ecg(tprn_cfg.c[i]))
    {
      ids2string(IDS_MMPMV, s2);
      h = (int)(tprn_amp[tprn_cfg.amp[i]] * tprn_amp_mul[tprn_cfg.c[i]] * TPRN_PAPER_DPM);
      sprintf(s, "%.1f %s", tprn_amp[tprn_cfg.amp[i]] * tprn_amp_mul[tprn_cfg.c[i]], s2);
    }
    else if (tprn_cfg.c[i] == CO2)
    {
      ids2string(IDS_MM_HG, s2);
     // h = (int)(tprn_amp[tprn_cfg.amp[i]] * tprn_amp_mul[tprn_cfg.c[i]] * TPRN_PAPER_DPM);
      h = 10 * TPRN_PAPER_DPM; // 10 mm (x2) or 5 mm (x1)
      if (tprn_cfg.amp[i] == TPRN_AMP_X1) h /= 2;
      sprintf(s, "50 %s", s2);
    }
    else
    {
      if (tprn_cfg.c[i] == SPO2 && cframe_getptr()->chanview[SPO2].chandata.my == MY_AUTO)
        ids2string(IDS_AUTO, s);
      else
        sprintf(s, "x%d", (int) (tprn_amp[tprn_cfg.amp[i]]));
      h = 0;
    }
    grph_textout(tprn_grph_fd_p, xw, y0[i]+10, s, strlen(s));

    //w = (int)(tprn_spd[tprn_cfg.spd] * TPRN_PAPER_DPM);

    if (h > 0)
    {
      grph_line(tprn_grph_fd_p, xw+100, y0[i]-h/2, xw+100, y0[i]+h/2);
      grph_line(tprn_grph_fd_p, xw+100+w, y0[i]-h/2, xw+100+w, y0[i]+h/2);
      grph_line(tprn_grph_fd_p, xw+100, y0[i]-h/2, xw+100+w, y0[i]-h/2);

      grph_line(tprn_grph_fd_p, xw+100-5, y0[i]+h/2, xw+100, y0[i]+h/2);
      grph_line(tprn_grph_fd_p, xw+100+w, y0[i]+h/2, xw+100+w+5, y0[i]+h/2);
    }
  }

  if (n > 0)
  {
    xw = xw + 100 + w + 5;
  }

  n = min(xw + TPRN_PAPER_DPM*1, pgdc->xres);
  for (x=0; x<n; x+=8)
  {
    for (y=0; y<pgdc->yres; y++)
    {
      a[y] = (((unsigned char*)(pgdc->addr))[x/8+y*((pgdc->xres+7)/8)]);
    }
    tprn_put_8data(a, pgdc->yres, 8);
  }

  return n;
}

static void tprn_flush_data(void)
{
  tprn_cfg_t tprn_cfg;
  int no;
  tprn_cfg_get(&tprn_cfg);
  no = ((chno_is_ecg(tprn_cfg.c[0]))?unit_isenabled(ECG):unit_isenabled(tprn_cfg.c[0])) ? 0 : 1;
  for (;;data_ptr_r++)
  {
    if ( abs(data_ptr_r - (int)data_ptr[no]) < 3) break;
    if (data_ptr_r >= data_max_res) data_ptr_r = 0;
    tprn_put_line(data_ptr_r);
  }
}

void tprn_push_data(int no, int * buf, int nPoints, unsigned long dwTimeMS)
{
  tprn_cfg_t tprn_cfg;
  int i, y0[TPRN_NUM_CURVES];
  double PixelPerSecond;
  double PixelPerPoint;
  float k;

  tprn_cfg_get(&tprn_cfg);

  if (no > tprn_cfg.n-1) return;

  k = 1.0f;
  if (tprn_cfg.c[no] == SPO2)
  {
    cframe_t * pcframe = cframe_getptr();
    k = (pcframe->chanview[SPO2].chandata.my != MY_AUTO) ? 1.0f : (float)0xffff/0xfff;
  }

  tprn_stored_data[no].dt[tprn_stored_data[no].wiptr] = dwTimeMS;
  tprn_stored_data[no].num[tprn_stored_data[no].wiptr] = nPoints;
  tprn_stored_data[no].wiptr += 1;
  if (tprn_stored_data[no].wiptr >= TPRN_STORED_DATA_SIZE) tprn_stored_data[no].wiptr = 0;
  for (i=0; i<nPoints; i++)
  {
    tprn_stored_data[no].data[tprn_stored_data[no].wdptr] = (int)((float)k*buf[i]);
    tprn_stored_data[no].wdptr += 1;
    if (tprn_stored_data[no].wdptr >= TPRN_STORED_DATA_SIZE) tprn_stored_data[no].wdptr = 0;
  }

  if (printing)
  {
    // YK: to do once on init and reset
    for (i=0; i<TPRN_NUM_CURVES; i++)
    {
      if (tprn_cfg.c[i] == TPRN_UNDEF_CURVE) continue;
      y0[i] = (i+1) * TPRN_PAPER_DW / (tprn_cfg.n + 1);
    }
    PixelPerSecond = (double)tprn_spd[tprn_cfg.spd] * TPRN_PAPER_DPM;
    PixelPerPoint = PixelPerSecond * ((double)dwTimeMS / nPoints / 1000);

    for (i=0; (unsigned int)i<nPoints; i++)
    {
      int yfrom, yto;

      if ((int)(data_ptr[no]+PixelPerPoint) >= data_max_res) break;

      // not needed (see "if" above
#if 0
      assert ((int)(data_ptr[no]+PixelPerPoint) < data_max_res);
#else
      if ( (data_ptr[no]+PixelPerPoint) >= data_max_res)
      {
        error("assert skipped: %s line %d\n", __FILE__, __LINE__);
      }
#endif

      yfrom = (int)(y0[no] + (tprn_v_old[no] * tprn_amp[tprn_cfg.amp[no]] * tprn_amp_mul[tprn_cfg.c[no]] * TPRN_PAPER_DPM / 1000) * (-1));
      yto   = (int)(y0[no] + ((int)((float)k*buf[i]) * tprn_amp[tprn_cfg.amp[no]] * tprn_amp_mul[tprn_cfg.c[no]] * TPRN_PAPER_DPM / 1000) * (-1));

      grph_line( tprn_grph_fd,
                 (int)(data_ptr[no]+0),
                 yfrom,
                 (int)(data_ptr[no]+PixelPerPoint),
                 yto
                );

      if (tprn_cfg.prnmask.width == TPRN_WIDTH_2 || tprn_cfg.prnmask.width == TPRN_WIDTH_3)
      {
         grph_line(tprn_grph_fd,
                  (int)(data_ptr[no]+0),
                  yfrom-1,
                  (int)(data_ptr[no]+PixelPerPoint),
                  yto-1
                 );
        grph_line(tprn_grph_fd,
                  (int)(data_ptr[no]+0)+1,
                  yfrom-1,
                  (int)(data_ptr[no]+PixelPerPoint)+1,
                  yto-1
                 );
      }
      if (tprn_cfg.prnmask.width == TPRN_WIDTH_3)
      {
        grph_line(tprn_grph_fd,
                  (int)(data_ptr[no]+0),
                  yfrom+1,
                  (int)(data_ptr[no]+PixelPerPoint),
                  yto+1
                 );
        grph_line(tprn_grph_fd,
                  (int)(data_ptr[no]+0)+1,
                  yfrom+1,
                  (int)(data_ptr[no]+PixelPerPoint)+1,
                  yto+1
                 );
      }

      tprn_v_old[no] = (int)((float)k*buf[i]);

      data_ptr[no] += PixelPerPoint;
    }
  }
}

void tprn_reset_grph(void)
{
  PGDC pgdc;
  tprn_cfg_t tcfg;
  int i, j;

  tprn_cfg_get(&tcfg);
  pgdc = (PGDC) tprn_grph_fd;

  if (tcfg.prnmask.grid)
  {
    unsigned char * line;
    pgdc = (PGDC)tprn_grph_fd;
    for (i=0; i<pgdc->yres; i++)
    {
      if (i % (10*TPRN_PAPER_DPM) == 0)
        line = cmline3;
      else
      if (i % TPRN_PAPER_DPM == 0)
        line = cmline2;
      else
      if (i % 4 == 0)
        line = cmline1;
      else
        line = cmline0;
      for (j=0; j<pgdc->xres; j+=10*TPRN_PAPER_DPM)
        memcpy((unsigned char*)pgdc->addr+i*pgdc->xres+j, line, 10*TPRN_PAPER_DPM);
    }
  }
  else
  {
    for (i=0; i<pgdc->yres; i++)
    {
      memset((unsigned char*)pgdc->addr+i*pgdc->xres, 0, pgdc->xres);
    }
  }
}

#if 0
static void tprn_check_packet(unsigned char * buf)
{
  unsigned char crc = 0, i;

  assert(buf[0] == TPRN_START);
  assert(buf[1] == TPRN_ID_DATA);
  assert(buf[2] == 48);

  for (i=0; i<TPRN_PACKET_SIZE-1; i++)
    crc += buf[i];

  assert(crc == buf[TPRN_PACKET_SIZE-1]);
}
#endif

static void tprn_flush_file_buffers(void)
{
  unsigned char buf[TPRN_PACKET_SIZE*50*TPRN_PAPER_DPM]; // 1 sec at 50 mm/s
 // unsigned char buf[TPRN_PACKET_SIZE];
  int nr, nw;
  tprn_cfg_t tprn_cfg;

  if (fd <= 0) return;

  tprn_cfg_get(&tprn_cfg);
  if (!tprn_cfg.prnmask.started)
  {
    sent_to_tprn = 0;
    return;
  }

  do
  {
    nr = min(sizeof(buf), (signed long)num_lines_to_send*packet_size - sent_to_tprn);
#ifndef ARM
    tprn_avail_size = nr;
#endif

#ifndef ARM
  //  assert( (num_lines_to_send*packet_size - sent_to_tprn) % packet_size == 0);
#endif

#if 0
    if (tprn_avail_size > TPRN_PACKET_SIZE)
      tprn_avail_size = (tprn_avail_size / TPRN_PACKET_SIZE) * TPRN_PACKET_SIZE;
#endif
    nr = min(tprn_avail_size, nr);

   // printf("nr1 = %d (%d)\n", nr, (int)(signed long)num_lines_to_send*packet_size - sent_to_tprn);
   // printf("avail = %d\n", fifo_avail(&fifo_tprn_data));

    nr = fifo_watch(&fifo_tprn_data, buf, nr);

#ifndef ARM
//    tprn_check_packet(buf);
#endif
  //  tprn_watch(buf, nr);
   // printf("nr2 = %d\n", nr);
    if (nr == 0)
    {
      tprn_flush_data();
     // nr = fifo_watch(&fifo_tprn_data, buf, nr);
    }
    nw = com_write(fd, (char*)buf, nr);
   // printf("nw = %d\n", nw);

   // nw = nr;
    if (nw > 0)
    {
      sent_to_tprn += nw;
#if 0
      assert ( fifo_pop(&fifo_tprn_data, nw) == nw);
#else
      if (fifo_pop(&fifo_tprn_data, nw) != nw)
      {
        error("assert skipped: %s line %d\n", __FILE__, __LINE__);
      }
#endif
     // tprn_skip(nw);
#ifdef ARM
      tprn_avail_size -= nw;
#endif
    }
//    else
//      perror("nw < 0: ");
   // printf("ostal = %d\n", (int)(signed long)num_lines_to_send*packet_size - sent_to_tprn);
    if (sent_to_tprn >= num_lines_to_send*packet_size)
    {
      tprn_stop();
      sent_to_tprn = 0;
      break;
    }
  } while (nr > 0 && nw > 0);
}

void tprn_start(void)
{
  tprn_cfg_t tprn_cfg;
  long num;
  PGDC pgdc;
  struct tm ltm;
  char s[200];
  ecg_data_t ecg_data;
  rect_t rc;
  int xx, i, j;
  unsigned char * line;

  if (fd <= 0)
  {
    alarm_set(TPRN_NOCONNECT);
  }

  if ((tprn_hw_status & 0x7) != 0)
  {
    // any errors in printer
    alarm_set_clr(TPRN_OPENED, tprn_hw_status & 0x01);   // printer opened
    alarm_set_clr(TPRN_PAPEROUT, tprn_hw_status & 0x02); // paper out
    alarm_set_clr(TPRN_OVERHEAT, tprn_hw_status & 0x04); // printer overheat
    debug("print stoped (tprn hw status = 0x%02X)\n", tprn_hw_status);
    tprn_status = 0;
    return;
  }

  tprn_cfg_get(&tprn_cfg);

  if ( fifo_pop(&fifo_tprn_data, fifo_avail(&fifo_tprn_data)) < 0)
  {
    assert(0);
  }
 // rptr = wptr = 0;

  num = tprn_flush_info();

  pgdc = (PGDC)tprn_grph_fd;

  // reset all old data
  for (i=0; i<pgdc->yres; i++)
  {
    if (tprn_cfg.prnmask.grid)
    {
      if (i % (10*TPRN_PAPER_DPM) == 0)
        line = cmline3;
      else
      if (i % TPRN_PAPER_DPM == 0)
        line = cmline2;
      else
      if (i % 4 == 0)
        line = cmline1;
      else
        line = cmline0;
      for (j=0; j<pgdc->xres; j+=10*TPRN_PAPER_DPM)
        memcpy((unsigned char*)pgdc->addr+i*pgdc->xres+j, line, 10*TPRN_PAPER_DPM);
      }
    else
    {
      memset((unsigned char*)pgdc->addr+i*pgdc->xres, 0, pgdc->xres);
    }
  }

  xx = tprn_spd[tprn_cfg.spd]*tprn_cfg.delay1*TPRN_PAPER_DPM;

  grph_line(tprn_grph_fd, xx, 0, xx, pgdc->yres-1);
 // grph_line(tprn_grph_fd, xx+1, 0, xx+1, pgdc->yres-1);
 // grph_line(tprn_grph_fd, 2+1, 0, 2+1, pgdc->yres-1);
  grph_filltriangle(tprn_grph_fd, xx, 0, xx+2, 2, xx+4, 0);
  grph_filltriangle(tprn_grph_fd, xx, pgdc->yres-1, xx+2, pgdc->yres-1-2, xx+4, pgdc->yres-1);

  grph_fillrect(tprn_grph_fd, xx+5, 10, 55, 16, 0x000000);
  grph_setfont(tprn_grph_fd, MONOS_11);

  read_hwclock_data(&ltm);
  sprintf(s, "%02d:%02d:%02d", ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
  grph_textout(tprn_grph_fd, xx+10, 10, s, strlen(s));

  if ( chno_is_ecg(tprn_cfg.c[0]) || chno_is_ecg(tprn_cfg.c[1]) )
  {
    unit_get_data(ECG, &ecg_data);
    switch (ecg_data.set_bits.tau)
    {
      case TAU_3200:
        if (ecg_data.set_bits.diag_f)
          ids2string(ecg_mode_ids[MODE_DIAGNOSTICS_F], s);
        else
          ids2string(ecg_mode_ids[MODE_DIAGNOSTICS], s);
        break;
      case TAU_320:
        ids2string(ecg_mode_ids[MODE_MONITORING], s);
        break;
      case TAU_160:
        ids2string(ecg_mode_ids[MODE_SURGERY], s);
       break;
      default:
       ids2string(IDS_UNDEF3, s);
       break;
    }

    rc.x0 = xx+10;
    rc.y0 = pgdc->yres - 20;
    grph_drawtext(tprn_grph_fd, s, -1, &rc, DT_CALCRECT);
    grph_fillrect(tprn_grph_fd, rc.x0-5, rc.y0, rc.x1-rc.x0+10, 16, 0x000000);
    grph_drawtext(tprn_grph_fd, s, -1, &rc, DT_LEFT);
  }

#if 1
  double data_ptr_;
  int dptr, iptr, no, ip;
  int dptr0[TPRN_NUM_CURVES], iptr0[TPRN_NUM_CURVES];
  double PixelPerSecond;
  double PixelPerPoint;
  unsigned int dwTimeMS, nPoints, totalTime;
  int y0[TPRN_NUM_CURVES], v_old[TPRN_NUM_CURVES];

  for (i=0; i<TPRN_NUM_CURVES; i++)
  {
    if (tprn_cfg.c[i] == TPRN_UNDEF_CURVE) continue;
   // y0[i] = (i+1) * TPRN_PAPER_DW / (tprn_cfg.n + 1);
    data_ptr[i] = xx;
    iptr0[i] = tprn_stored_data[i].wiptr-1;
    dptr0[i] = tprn_stored_data[i].wdptr-1;
   // if (iptr0[i] < 0) iptr0[i] += TPRN_STORED_DATA_SIZE;
    if (dptr0[i] < 0) dptr0[i] += TPRN_STORED_DATA_SIZE;
    v_old[i] = tprn_v_old[i] = tprn_stored_data[i].data[dptr0[i]];
  }

  printing = 1;

  // move after printing set, for making behing loop faster
  for (i=0; i<TPRN_NUM_CURVES; i++)
  {
    if (tprn_cfg.c[i] == TPRN_UNDEF_CURVE) continue;
    y0[i] = (i+1) * TPRN_PAPER_DW / (tprn_cfg.n + 1);

  }

  for (no=0; no<tprn_cfg.n; no++)
  {
    data_ptr_ = xx;
    iptr = iptr0[no];
    dptr = dptr0[no];
    if ( !((chno_is_ecg(tprn_cfg.c[no]))?unit_isenabled(ECG):unit_isenabled(tprn_cfg.c[no])) )
    {
      ids2string(IDS_NO_DATA, s);
      grph_textout(tprn_grph_fd, 0+30, y0[no]-5, s, strlen(s));
     // data_ptr[no] = xx;
      continue;
    }
   // tprn_stored_data[no].riptr = iptr;
   // tprn_stored_data[no].rdptr = dptr;
   // tprn_v_old[no] = tprn_stored_data[no].data[dptr];
   // v_old = tprn_v_old[no];
    totalTime = 0;
    for (ip=0; ip<TPRN_STORED_DATA_SIZE; ip++)
    {
      iptr = iptr0[no]-ip;
      if (iptr < 0) iptr += TPRN_STORED_DATA_SIZE;
      if (dptr < 0) dptr += TPRN_STORED_DATA_SIZE;
      dwTimeMS = tprn_stored_data[no].dt[iptr];
      nPoints = tprn_stored_data[no].num[iptr];
      PixelPerSecond = (double)tprn_spd[tprn_cfg.spd] * TPRN_PAPER_DPM;
      PixelPerPoint = PixelPerSecond * ((double)dwTimeMS / nPoints / 1000);
      totalTime += dwTimeMS;
      for (i=0; i<nPoints; i++)
      {
        int yfrom, yto;

        yfrom = y0[no] + (v_old[no] * tprn_amp[tprn_cfg.amp[no]] * tprn_amp_mul[tprn_cfg.c[no]] * TPRN_PAPER_DPM / 1000) * (-1);
        yto   = y0[no] + (tprn_stored_data[no].data[dptr] * tprn_amp[tprn_cfg.amp[no]] * tprn_amp_mul[tprn_cfg.c[no]] * TPRN_PAPER_DPM / 1000) * (-1);

        grph_line( tprn_grph_fd,
                   (int)(data_ptr_+0),
                   yfrom,
                   (int)(data_ptr_-PixelPerPoint),
                   yto
                 );

        if (tprn_cfg.prnmask.width == TPRN_WIDTH_2 || tprn_cfg.prnmask.width == TPRN_WIDTH_3)
        {
          grph_line( tprn_grph_fd,
                     (int)(data_ptr_+0),
                     yfrom-1,
                     (int)(data_ptr_-PixelPerPoint),
                     yto-1
                   );

          grph_line( tprn_grph_fd,
                     (int)(data_ptr_+0)+1,
                     yfrom-1,
                     (int)(data_ptr_-PixelPerPoint)+1,
                     yto-1
                   );
        }
        if (tprn_cfg.prnmask.width == TPRN_WIDTH_3)
        {
          grph_line( tprn_grph_fd,
                     (int)(data_ptr_+0),
                     yfrom+1,
                     (int)(data_ptr_-PixelPerPoint),
                     yto+1
                   );

          grph_line( tprn_grph_fd,
                     (int)(data_ptr_+0)+1,
                     yfrom+1,
                     (int)(data_ptr_-PixelPerPoint)+1,
                     yto+1
                   );
        }

        v_old[no] = tprn_stored_data[no].data[dptr];

        dptr --;
        if (dptr < 0) dptr += TPRN_STORED_DATA_SIZE;
        data_ptr_ -= PixelPerPoint;
      }
    }
   // data_ptr[no] = xx;
  }
#endif

  data_ptr_r = 0;

 // tprn_flush_data();

 // num_lines_to_send = num;
  if (tprn_cfg.n > 0)
    num_lines_to_send = num + tprn_spd[tprn_cfg.spd]*(tprn_cfg.delay1+tprn_cfg.delay2)*TPRN_PAPER_DPM;
  else
    num_lines_to_send = num;
//printf("num_lines_to_send = %d\n", (int)num_lines_to_send);

  tprn_cfg.prnmask.started = 1;
  tprn_cfg_set(&tprn_cfg);

  tprn_status = 1; // ready to print
}

void tprn_stop(void)
{
  tprn_cfg_t tprn_cfg;

  tprn_cfg_get(&tprn_cfg);
  tprn_cfg.prnmask.started = 0;
  printing = 0;
  tprn_cfg_set(&tprn_cfg);

  num_lines_to_send = 0;

 // tprn_flush_file_buffers(); // clear static variable cnt in tprn_flush_file_buffers
  sent_to_tprn = 0; // replaces with file static variable, prevent recursion

  sched_start(SCHED_ANY, 600, tprn_feed, SCHED_DO_ONCE);

  tprn_status = 0;
}

static void tprn_feed(void)
{
  // feed paper (15 mm)
  tprn_command(TPRN_ID_SETUP, 0x40, 15, 0x00, 0x00);
}

static int tprn_process_msg(unsigned char * buf)
{
  unsigned char crc;
  int i;
  static unsigned char old_data1 = 0xff;

/*
#ifdef TPRN_DEBUG
  fprintf(stdout, "IN: ");
  for (i=0; i<TPRN_MSG_SIZE; i++)
  {
    fprintf(stdout, "%02X ", buf[i]);
  }
  fprintf(stdout, "\n");
  fflush(stdout);
#endif
*/

  if (buf[0] != TPRN_START)
  {
    error("tprn_process_msg: SOF error\n");
    return 0;
  }
  if (buf[1] != TPRN_ID_STAT)
  {
    error("tprn_process_msg: ID error\n");
    return 0;
  }
  if (buf[2] != 4)
  {
    error("tprn_process_msg: LEN error\n");
    return 0;
  }
  crc = 0;
  for (i=0; i<TPRN_MSG_SIZE-1; i++)
  {
    crc += buf[i];
  }
  if (buf[TPRN_MSG_SIZE-1] != crc)
  {
    error("tprn_process_msg: CRC error\n");
    return 0;
  }

#ifdef TPRN_DEBUG
  fprintf(stdout, "IN: ");
  for (i=0; i<TPRN_MSG_SIZE; i++)
  {
    fprintf(stdout, "%02X ", buf[i]);
  }
  fprintf(stdout, "\n");
  fflush(stdout);
#endif

  if (tprn_status == 1)
  {
    // printer powered on
    tprn_status ++;
  }

  if (tprn_status == 2)
  {
    tprn_cfg_t tprn_cfg;

    tprn_cfg_get(&tprn_cfg);
    if (1 << (tprn_cfg.spd+1) == (buf[3] & 0x0E)) // data0
    {
      debug("tprn print speed asserted\n");
      tprn_status ++;
    }
  }

  // process warning messages from tprn
  if (buf[4] != old_data1)
  {
    alarm_set_clr(TPRN_OPENED, buf[4] & 0x01);   // printer opened
    alarm_set_clr(TPRN_PAPEROUT, buf[4] & 0x02); // paper out
    alarm_set_clr(TPRN_OVERHEAT, buf[4] & 0x04); // printer overheat

    // stop print if any
    if (1)
    {
    }

    old_data1 = buf[4];
    tprn_hw_status = buf[4];
  }

  tprn_avail_size = (buf[6] << 8) | buf[5];

  tprn_avail_size = (tprn_avail_size < 8000) ? 0 : tprn_avail_size;

 // printf("tprn_avail_size = %d\n", tprn_avail_size);

  return 1;
}

static void tprn_add_byte(unsigned char b)
{
  tprnin_buf[tprnin_ptr] = b;
  if (tprnin_buf[0] == TPRN_START)
    tprnin_ptr ++;

  if (tprnin_ptr == TPRN_MSG_SIZE)
  {
    if ( tprn_process_msg(tprnin_buf) )
    {
      tprnin_ptr = 0;
    }
#if 0
    else
    {
      int i;
      for (i=0+1; i<TPRN_MSG_SIZE; i++)
      {
        if (tprnin_buf[i] == TPRN_START) break;
      }
      if (i != TPRN_MSG_SIZE)
      {
        memmove(tprnin_buf, &tprnin_buf[i], TPRN_MSG_SIZE-i);
      }
      tprnin_ptr = TPRN_MSG_SIZE-i;
    }
#endif
    tprnin_ptr = 0;
  }

  if (tprnin_ptr >= TPRN_MSG_SIZE) // never true
  {
    error("tprnio_add_byte: overflow %d\n", tprnin_ptr);
    tprnin_ptr = 0;
  }
}

void tprn_command(int id, unsigned char data0, unsigned char data1, unsigned char data2, unsigned char data3)
{
  unsigned char buf[8], crc, i;

  if (fd <= 0) return;

  buf[0] = TPRN_START;     // start
  buf[1] = TPRN_ID_SETUP;  // id
  buf[2] = 4;              // data len
  buf[3] = data0;          // data
  buf[4] = data1;
  buf[5] = data2;
  buf[6] = data3;

  crc = 0x00;
  for (i=0; i<7; i++)
   crc += buf[i];
  buf[7] = crc; // crc

  com_write(fd, (char*)buf, 8);

#ifdef TPRN_DEBUG
  fprintf(stdout, "OUT: ");
  for (i=0; i<8; i++)
  {
    fprintf(stdout, "%02X ", buf[i]);
  }
  fprintf(stdout, "\n");
  fflush(stdout);
#endif
}

void tprn_stepit(void)
{
  unsigned char buf[300];
  int nr, i;
  tprn_cfg_t tprn_cfg;
  static int cnt = 0;
#ifdef ARM
  unsigned char b;
#endif

  // to disable warnings
  cnt = cnt;

  // process in data
  do
  {
    nr = com_read(fd, (char*)buf, sizeof(buf));
    if (nr > 0)
    {
      for (i=0; i<nr; i++)
      {
        tprn_add_byte(buf[i]);
      }
    }
  }
  while (nr > 0);

  tprn_cfg_get(&tprn_cfg);

  switch (tprn_status)
  {
    case 0:
      cnt = 0;
     // crbpio_send_msg(CRBP_SET_POWER, DEV_MASK_TPRN, 0x00, 0x00, 0x00); // printer off
      return;
    case 1:
      debug("power on printer\n");
#ifdef ARM
 /*     powerman_status_get(&b);
      if (b & DEV_MASK_TPRN)
      {
        tprn_status ++;
        cnt = 0;
      }
      else
      {
        crbpio_send_msg(CRBP_SET_POWER, DEV_MASK_TPRN, 0x00, 0x01, 0x00); // tprn on
      }
      cnt ++;
      if (cnt * TPRN_TS_PERIOD_MS > 2000)
      {
        debug("power on printer timeout\n");
        alarm_set(TPRN_NOCONNECT);
        tprn_stop();
        cnt = 0;
      }*/
#else
      tprn_status ++;
#endif
      error("skip check for power off printer\n");
      tprn_status ++;
      cnt = 0;
      break;
    case 2:
      debug("set up printer cfg\n");
#ifdef ARM
      cnt ++;
      // thin out requests
      b = (1 << (tprn_cfg.spd + 1)); // 12.5 -> 0x02, 25 -> 0x04, 50 -> 0x08
      tprn_command(TPRN_ID_SETUP, b, 0x00, 0x00, 0x00);
      if (cnt * TPRN_TS_PERIOD_MS > 2000)
      {
        debug("set up printer timeout\n");
        tprn_cfg.prnmask.started = 0;
        printing = 0;
        tprn_status = 0;
       // tprn_stop();
        cnt = 0;
        alarm_set(TPRN_NOCONNECT);
      }
#else
      if (fd > 0) tprn_status ++;
      else
      {
        debug("print stopped (invalid comm port)\n");
        tprn_cfg.prnmask.started = 0;
        printing = 0;
        tprn_status = 0;
      }
#endif
      break;
    case 3:
      if ((tprn_hw_status & 0x7) != 0)
      {
        debug("tprn_hw_status = %d, stop print\n", tprn_hw_status);
        // any errors in printer
        alarm_set_clr(TPRN_OPENED, tprn_hw_status & 0x01);   // printer opened
        alarm_set_clr(TPRN_PAPEROUT, tprn_hw_status & 0x02); // paper out
        alarm_set_clr(TPRN_OVERHEAT, tprn_hw_status & 0x04); // printer overheat
        tprn_cfg.prnmask.started = 0;
        printing = 0;

        // force printer reset
        cnt = 0;
        tprn_status = 0;
        break;
      }
      debug("start data transmittion to printer\n");
      alarm_clr(TPRN_NOCONNECT);
#ifdef TPRN_TRANSPARENT_INKS
      debug("attention: printer has transparent inks!!!\n");
#endif
      tprn_status ++;
      break;
    case 4:
      tprn_flush_file_buffers(); // leave func, not break, to don't save old tprn_cfg
      return;
    default:
      error("%s: undefined case %d\n", __FUNCTION__, tprn_status);
      break;
  }
  tprn_cfg_set(&tprn_cfg);
}

void tprn_cfg_save(void)
{
  tprn_cfg_t tcfg;

  tprn_cfg_get(&tcfg);
  tcfg.prnmask.started = 0; // fake !
  printing = 0;
  if ( stconf_write(STCONF_TPRN, &tcfg, sizeof(tprn_cfg_t)) > 0 );
}


