#include <string.h>
#include <stdio.h>
#include "csio_typedefs.h"
#include "debug.h"
#include "dproc.h"

static unsigned char ena[CSIO_ID_MAXNUM];

void dproc_init(void)
{
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
  for (int i=0; i<CSIO_ID_MAXNUM; i++)
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
      csio_debug_info_t * p = (csio_debug_info_t *) &buf[16];
      fprintf(stdout, "%s", p->payload);
    }
    break;
    case CSIO_ID_RESP_C:
    {
      csio_respdata_c_t * p = (csio_respdata_c_t*) &buf[16];
      static int sync_prev = 0;
      if (p->sync-sync_prev != 1 && (sync_prev-p->sync != 255))
      {
     // printf("%02X %02X\n", buf[16], sync_prev);
        error("RESP C SYNC ERROR\n");
      }
      sync_prev = p->sync;
      debug("rpg\t%d\n", p->rpg);
    }
    break;
    case CSIO_ID_ECG_C:
    {
      csio_ecgdata_c_t * p = (csio_ecgdata_c_t*) &buf[16];
  //    printf("p[0]=%02X\n", buf[16]);
      static int sync_prev = 0;
      if (p->sync-sync_prev != 1 && (sync_prev-p->sync != 255))
      {
     // printf("%02X %02X\n", buf[16], sync_prev);
        error("ECG C SYNC ERROR\n");
      }
      sync_prev = p->sync;
      short i = p->data[0], ii = p->data[1];
      debug("ECG: \tI %d \tII %d \tIII %d \taVR %d \taVL %d \taVF %d \n", i, ii, ii-i, (i-ii)/2, (2*i-ii)/2, i/2);
    }
    break;
    default:
      break;
  }
}
