#include <string.h>
#include <stdio.h>
#include "csio_typedefs.h"
#include "debug.h"
#include "dproc.h"

static unsigned char ena[CSIO_ID_MAXNUM];

void dproc_init(void)
{
  int i;
  memset(ena, 0, sizeof(ena));
  FILE * f = fopen("cs_demo.ini", "rt");
  if (!f)
  {
    error("unable to read сап file\n");
    return;
  }
  char s[200];
  int chan = 0;
  while (!feof(f))
  {
    if ( fgets(s, sizeof(s), f) )
    {
      char * ps = strstr(s, "=");
      if (!ps) continue;
      ps ++;
      while (*ps == ' ' || *ps == '\t') ps ++;
      if (*ps == '1')
        ena[chan] = 1;
      chan ++;
    }
  }
  for (i=0; i<CSIO_ID_MAXNUM; i++)
  {
    debug("channel %d\t%s\n", i, ena[i] ? "ENA" : "DIS");
  }
}

void dproc_packet(int id, unsigned long ts, unsigned char * buf, int len)
{
  if (ena[id] == 0) return;
  if (id >= CSIO_ID_MAXNUM) return;

  switch (id)
  {
    case CSIO_ID_DEBUG:
    {
      csio_debug_info_t * p = (csio_debug_info_t *) buf;
      fprintf(stdout, "%s", p->payload);
    }
    break;
    case CSIO_ID_RESP_C:
    {
      csio_respdata_c_t * p = (csio_respdata_c_t*) buf;
      static int sync_prev = 0;
      if (p->sync-sync_prev != 1 && (sync_prev-p->sync != 255))
      {
        error("RESP C SYNC ERROR\n");
      }
      sync_prev = p->sync;
      debug("rpg\t%d\n", p->rpg);
    }
    break;
    case CSIO_ID_ECG_C:
    {
      csio_ecgdata_c_t * p = (csio_ecgdata_c_t*) buf;
      static int sync_prev = 0;
      if (p->sync-sync_prev != 1 && (sync_prev-p->sync != 255))
      {
        error("ECG C SYNC ERROR\n");
      }
      sync_prev = p->sync;
      short i = p->data[0], ii = p->data[1];
      debug("ECG: \tI %d \tII %d \tIII %d \taVR %d \taVL %d \taVF %d \n", i, ii, ii-i, (i-ii)/2, (2*i-ii)/2, i/2);
    }
    break;
    case CSIO_ID_SPO2_C:
    {
      csio_spo2data_c_t * p = (csio_spo2data_c_t*) buf;
      static int sync_prev = 0;
      if (p->sync-sync_prev != 1 && (sync_prev-p->sync != 255))
      {
        error("SPO2 C SYNC ERROR\n");
      }
      sync_prev = p->sync;
      debug("fpg: %d (mode %d)\n", p->fpg, p->mode);
    }
    break;
    case CSIO_ID_CO2_C:
    {
      csio_co2data_c_t * p = (csio_co2data_c_t*) buf;
      static int sync_prev = 0;
      if (p->sync-sync_prev != 1 && (sync_prev-p->sync != 127))
      {
        error("CO2 C SYNC ERROR\n");
      }
      sync_prev = p->sync;
      debug("co2: %d\n", p->data);
    }
    break;
    case CSIO_ID_ECG_D:
    {
      csio_ecgdata_d_t * p = (csio_ecgdata_d_t*) buf;
      debug("ecg: hr=\t%d; \t\tst= \t%d \t%d \t%d \t%d \t%d \t%d \t%d\n", p->hr, p->st[0], p->st[1], p->st[2], p->st[3], p->st[4], p->st[5], p->st[6]);
    }
    break;
    case CSIO_ID_SPO2_D:
    {
      csio_spo2data_d_t * p = (csio_spo2data_d_t*) buf;
      debug("spo2: pulse=\t%d; \t\tsat= \t%d; \t\tstolb=\t%d\n", p->hr, p->spo2, p->stolb);
    }
    break;
    case CSIO_ID_NIBP_D:
    {
      csio_nibpdata_d_t * p = (csio_nibpdata_d_t*) buf;
      debug("nibp: %d/%d (%d); hr=%d, infl=%d\n", p->sd, p->dd, p->md, p->hr, p->infl);
    }
    break;
    default:
      break;
  }
}
