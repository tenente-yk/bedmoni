/*! \file csio.c
    \brief Central station i/o.
*/

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "bedmoni.h"
#include "utils.h"
#include "mframe.h"
#include "unit.h"
#include "powerman.h"
#include "alarm.h"

#include "udp.h"
#include "csio.h"

static unsigned char csio_outbuf[CSIO_OUTBUF_SIZE];
static unsigned long orptr = 0;
static unsigned long owptr = 0;
static int csio_is_init = 0;

static void csio_flush(void);

void csio_init(void)
{
#if defined (UDP_BEDMONI_SERVER)
  udp_server_init();
#endif
#if defined (UDP_BEDMONI_CLIENT)
  udp_client_init();
#endif

  orptr = 0;
  owptr = 0;

  csio_is_init = 1;
}

void csio_deinit(void)
{
#if defined (UDP_BEDMONI_SERVER)
  udp_server_deinit();
#endif
#if defined (UDP_BEDMONI_CLIENT)
  udp_client_deinit();
#endif

  csio_is_init = 0;
}

int csio_read(void * pbuf, int len)
{
  return udp_read(pbuf, len);
}

int csio_write(const void * pbuf, int len)
{
  return udp_write(pbuf, len);
}

int csio_send_msg(int msgid, const void * data, int len)
{
  unsigned char msgout[256], outlen, crc;
  int i;
  unsigned long ts;
  static unsigned long ts_base = 0;
  outlen = 0;

  if (!csio_is_init) return -1;
  if (!data || len<0) return -1;

  msgout[0] = CS_OUT_START;
  msgout[1] = 0x00; // reserved
#ifndef ARM
  if (serial_no == SERIAL_NO_UNINIT)
  {
    serial_no = 0;
  }
#endif
  memcpy(&msgout[2], &serial_no, sizeof(int));
  msgout[6] = 0x77; // reserved
  msgout[7] = 0x88; // reserved
  ts = gettimemillis();
  memcpy(&msgout[8], &ts_base, sizeof(unsigned long));
  msgout[12] = msgid & 0xFF;
  msgout[13] = (msgid >> 8) & 0xFF;
  msgout[14] = len & 0xFF;
  msgout[15] = (len >> 8) & 0xFF;
  memcpy(&msgout[16], data, len);

  crc = 0x00;
  for (i=1; i<16+len; i++)
  {
    crc+=msgout[i];
  }
  msgout[16+len] = crc;
  msgout[16+len+1] = CS_OUT_END;
  outlen = 16+len+2;

  if (owptr+outlen < CSIO_OUTBUF_SIZE)
  {
    memcpy(&csio_outbuf[owptr], msgout, outlen);
    owptr += outlen;
  }
  else
  {
    memcpy(&csio_outbuf[owptr], msgout, CSIO_OUTBUF_SIZE-owptr);
    memcpy(&csio_outbuf[0], &msgout[CSIO_OUTBUF_SIZE-owptr], outlen-(CSIO_OUTBUF_SIZE-owptr));
    owptr = outlen-(CSIO_OUTBUF_SIZE-owptr);
  }

  if (ts - ts_base >= 100)
  {
    csio_flush();
    ts_base = ts;
  }

  return 0;
}

static void csio_flush(void)
{
  const int packet_max_size = 400; // 500 or higher occurs transmit. errors
  int outlen, nw_total, nw, i;
  unsigned char * pbuf = 0;

  if (!udp_isready())
  {
    orptr = owptr;
    alarm_set(SYS_CS_NOCONNECT);
  }
  else
  {
    alarm_clr(SYS_CS_NOCONNECT);
  }

#if 1
// TODO: remove this when been tested with okay
  assert(orptr < CSIO_OUTBUF_SIZE);
  assert(owptr < CSIO_OUTBUF_SIZE);
#endif

  // flush collected data
  while (orptr != owptr && !exit_flag)
  {
    pbuf = &csio_outbuf[orptr];
    if (orptr <= owptr)
    {
      outlen = owptr - orptr;
    }
    else
    {
      outlen = - orptr + CSIO_OUTBUF_SIZE + owptr;
    }
    if (outlen > packet_max_size) outlen = packet_max_size;
    if (orptr + outlen > CSIO_OUTBUF_SIZE) outlen = CSIO_OUTBUF_SIZE - orptr;

#define NUM_RETRY_ATTEMPTS    5
    for(i=0,nw_total=0;i<NUM_RETRY_ATTEMPTS;i++)
    {
      nw = udp_write(pbuf+nw_total, outlen-nw_total);
      if (nw >= 0)
      {
        nw_total += nw;
        i = 0;
      }
      if (nw_total == outlen) break;
    }

    if (nw_total != outlen)
    {
      error("%s: not all bytes sent to CS\n", __FUNCTION__);
    }
   // assert(nw_total == outlen);

    orptr += nw_total;
    if (orptr >= CSIO_OUTBUF_SIZE) orptr -= CSIO_OUTBUF_SIZE;
  }
}

void csio_send_slow_data(void)
{
  int i,n;
  unsigned long v;

  // UNITs' DATA
  ecg_data_t  ecg_data;
  spo2_data_t spo2_data;
  nibp_data_t nibp_data;
  resp_data_t resp_data;
  t1t2_data_t t1t2_data;
  co2_data_t  co2_data;
  unit_get_data(ECG,  &ecg_data);
  unit_get_data(SPO2, &spo2_data);
  unit_get_data(NIBP, &nibp_data);
  unit_get_data(RESP, &resp_data);
  unit_get_data(T1T2, &t1t2_data);
  unit_get_data(CO2,  &co2_data);

  // ECG D
  csio_ecgdata_d_t csio_ecgdata_d;
  memset(&csio_ecgdata_d, 0, sizeof(csio_ecgdata_d_t));
  csio_ecgdata_d.hr = ecg_data.hr;
  csio_ecgdata_d.hr_min = ecg_data.hr_min;
  csio_ecgdata_d.hr_max = ecg_data.hr_max;
  n = (7<NUM_ECG) ? 7 : NUM_ECG;
  for (i=0; i<n; i++)
  {
    csio_ecgdata_d.st[i] = ecg_data.st[i];
    csio_ecgdata_d.st_min[i] = ecg_data.st_min[i];
    csio_ecgdata_d.st_max[i] = ecg_data.st_max[i];
  }
  csio_ecgdata_d.hr_src = ecg_data.hr_src;
  csio_ecgdata_d.j_shift = ecg_data.j_shift;
  csio_send_msg(CSIO_ID_ECG_D, &csio_ecgdata_d, sizeof(csio_ecgdata_d_t));

  // SPO2 D
  csio_spo2data_d_t csio_spo2data_d;
  memset(&csio_spo2data_d, 0, sizeof(csio_spo2data_d_t));
  csio_spo2data_d.spo2 = spo2_data.spo2;
  csio_spo2data_d.spo2_min = spo2_data.spo2_min;
  csio_spo2data_d.spo2_max = spo2_data.spo2_max;
  csio_spo2data_d.hr = spo2_data.hr;
  csio_spo2data_d.hr_min = spo2_data.hr_min;
  csio_spo2data_d.hr_max = spo2_data.hr_max;
  csio_spo2data_d.stolb = spo2_data.stolb;
  csio_send_msg(CSIO_ID_SPO2_D, &csio_spo2data_d, sizeof(csio_spo2data_d_t));

  // NIBP D
  csio_nibpdata_d_t csio_nibpdata_d;
  memset(&csio_nibpdata_d, 0, sizeof(csio_nibpdata_d_t));
  csio_nibpdata_d.sd = nibp_data.sd;
  csio_nibpdata_d.sd_min = nibp_data.sd_min;
  csio_nibpdata_d.sd_max = nibp_data.sd_max;
  csio_nibpdata_d.dd = nibp_data.dd;
  csio_nibpdata_d.dd_min = nibp_data.dd_min;
  csio_nibpdata_d.dd_max = nibp_data.dd_max;
  csio_nibpdata_d.md = nibp_data.md;
  csio_nibpdata_d.md_min = nibp_data.md_min;
  csio_nibpdata_d.md_max = nibp_data.md_max;
  csio_nibpdata_d.infl = nibp_data.infl;
  csio_nibpdata_d.meas_interval = nibp_data.meas_interval;
  csio_nibpdata_d.hr = nibp_data.hr;
  memcpy(&csio_nibpdata_d.trts, &nibp_data.trts_meas, sizeof(trts_t));
  csio_send_msg(CSIO_ID_NIBP_D, &csio_nibpdata_d, sizeof(csio_nibpdata_d_t));

  // RESP D
  csio_respdata_d_t csio_respdata_d;
  memset(&csio_respdata_d, 0, sizeof(csio_respdata_d_t));
  csio_respdata_d.br = resp_data.br;
  csio_respdata_d.br_min = resp_data.br_min;
  csio_respdata_d.br_max = resp_data.br_max;
  csio_respdata_d.ap = resp_data.ap;
  csio_respdata_d.ap_max = resp_data.ap_max;
  csio_respdata_d.lead = ((ecgmset_bits_t*)&ecg_data.set)->breath_ch;
  csio_send_msg(CSIO_ID_RESP_D, &csio_respdata_d, sizeof(csio_respdata_d_t));

  // T1T2 D
  csio_t1t2data_d_t csio_t1t2data_d;
  memset(&csio_t1t2data_d, 0, sizeof(csio_t1t2data_d_t));
  csio_t1t2data_d.t1 = t1t2_data.t1;
  csio_t1t2data_d.t1_min = t1t2_data.t1_min;
  csio_t1t2data_d.t1_max = t1t2_data.t1_max;
  csio_t1t2data_d.t2 = t1t2_data.t2;
  csio_t1t2data_d.t2_min = t1t2_data.t2_min;
  csio_t1t2data_d.t2_max = t1t2_data.t2_max;
  csio_t1t2data_d.dt_max = t1t2_data.dt_max;
  csio_send_msg(CSIO_ID_T1T2_D, &csio_t1t2data_d, sizeof(csio_t1t2data_d_t));

  // CO2 D
  csio_co2data_d_t csio_co2data_d;
  memset(&csio_co2data_d, 0, sizeof(csio_co2data_d_t));
  csio_co2data_d.etco2 = co2_data.etco2;
  csio_co2data_d.etco2_min = co2_data.etco2_min;
  csio_co2data_d.etco2_max = co2_data.etco2_max;
  csio_co2data_d.ico2 = co2_data.ico2;
  csio_co2data_d.ico2_max = co2_data.ico2_max;
  csio_co2data_d.br = co2_data.br;
  csio_co2data_d.br_min = co2_data.br_min;
  csio_co2data_d.br_max = co2_data.br_max;
  csio_send_msg(CSIO_ID_CO2_D, &csio_co2data_d, sizeof(csio_co2data_d_t));

  // ALARMS
  csio_alarms_t csio_alarms;
  memset(&csio_alarms, 0, sizeof(csio_alarms_t));
  for (i=0; i<NUM_ALARMS; i++)
  {
    if ((alarm_msg[i].state == MSG_SET && alarm_msg[i].fixed == MSG_NOFIX) ||
        (alarm_msg[i].state != MSG_CLR_FIXED && alarm_msg[i].fixed == MSG_FIX)
       )
    {
      csio_alarms.set[i/8] |= (1 << (i&7));
    }
    if (alarm_msg[i].enabled == SND_ENABLED)
    {
      csio_alarms.ena[i/8] |= (1 << (i&7));
    }
    csio_alarms.stat[i/8] |= (((alarm_msg[i].state == MSG_SET) ? 1 : 0) << (i&7));
  }
  csio_send_msg(CSIO_ID_ALARMS, &csio_alarms, sizeof(csio_alarms_t));

  // PAT
  csio_patient_t csio_patient;
  pat_pars_t pat_pars;
  memset(&csio_patient, 0, sizeof(csio_patient_t));
  pat_get(&pat_pars);
  csio_patient.type = pat_pars.type;
  csio_patient.bedno = pat_pars.bedno;
  csio_patient.cardno = pat_pars.cardno;
  csio_patient.w = pat_pars.w;
  csio_patient.h = pat_pars.h;
  csio_send_msg(CSIO_ID_PAT, &csio_patient, sizeof(csio_patient_t));

  // MONICFG
  csio_monicfg_t csio_monicfg;
  memset(&csio_monicfg, 0, sizeof(csio_monicfg_t));
  int r;
  for (i=0; i<NUM_VIEWS; i++)
  {
    r = unit_isenabled(i);
    if (r < 0) continue;
    if (r)
    {
      csio_monicfg.unitmask |= (1 << i);
    }
  }
  csio_monicfg.demomask = demo_mode;
  powerman_stat_get(&v);
  csio_monicfg.powerstat = v;
  csio_send_msg(CSIO_ID_MONICFG, &csio_monicfg, sizeof(csio_monicfg_t));
}

#if 0
//****************************************************************************
// incoming messages part

static unsigned char packet[200];
static int packet_ptr = 0;
static short packet_len = -1;

static int process_packet(unsigned char * buf)
{
  unsigned short id;
  short len;
  unsigned long serial_no_received;
  unsigned char crc;
  int i;

  if (buf[0] != CS_IN_START)
  {
    error("cs indata start error 0x%02X\n", buf[0]);
    return 0;
  }
  serial_no_received = (buf[5]<<24) | (buf[4]<<16) | (buf[3]<<8) | (buf[2]<<0);
  if (serial_no != serial_no_received)
  {
    // message not for this monitor
    return 0;
  }
  id  = (buf[9]<<8)  | buf[8];
  len = (buf[11]<<8) | buf[10];
  if (len > 100)
  {
    error("cs indata length error %d\n", len);
    return 0;
  }
  crc = 0;
  for (i=1;i<12+len;i++)
  {
    crc += buf[i];
  }
  if (crc != buf[12+len+1])
  {
    error("cs indata crc error 0x%02X 0x%02X\n", crc, buf[12+len+1]);
    return 0;
  }
  if (buf[12+len+2] != CS_IN_END)
  {
    error("cs indata end error 0x%02X\n", buf[0]);
    return 0;
  }
  return 1;
}

static void csio_process_data(unsigned char *pp, int len)
{
  int i;
  for (i=0;i<len;i++)
  {
    if (pp[i]==CS_IN_START && packet_ptr == 0)
    {
      // SOF detected
      packet[0] = pp[i];
      packet_ptr ++;
      continue;
    }
    if (packet_ptr == 0) continue;
    packet[packet_ptr++] = pp[i];
    if (packet_ptr == 12)
    {
      packet_len = (packet[11]<<8) | packet[10];
      if (packet_len < 0 || packet_len > 200)
      {
        packet_ptr = 0;
        packet_len = -1;
      }
      continue;
    }
    if ( packet_len > 0 && (packet_ptr == 15+packet_len) )
    {
      if (process_packet(packet))
      {
        packet_ptr = 0;
        packet_len = -1;
      }
      else
      {
        packet_ptr = 0;
        packet_len = -1;
      }
      continue;
    }
    if (packet_ptr >= sizeof(packet))
    {
      // reset parser
      packet_ptr = 0;
      packet_len = -1;
    }
  }
}
#endif

void csio_update(void)
{
#if 0
  unsigned char buf[300];
  int nr, i;

  do
  {
    nr = udp_read(buf, sizeof(buf));
    if (nr > 0)
    {
      csio_process_data((char*)buf[i], nr);
    }
  }
  while (nr > 0);
#endif
}

