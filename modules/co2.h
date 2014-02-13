#ifndef __CO2_H
#define __CO2_H

//#define CO2_DEBUG

#define CO2_MAX_NUM_CMDS_IN_QUEUE  20

#define NUM_CO2_BAL_GASES          3

#define CO2_CMD_WAVEFORM           0x80
#define CO2_CMD_ZERO               0x82
#define CO2_CMD_SETGET             0x84

#define CO2_GET_SENSOR_PARTNO      18

// see appendix A (capnostat5 UG 1015115DS1)
#pragma pack(1)
typedef struct
{
 // MLB
  unsigned char neg_co2_err     : 1;
  unsigned char check_adapter   : 1;
  unsigned char breath_ok       : 1;
  unsigned char out_of_range    : 1;
  unsigned char not_ready_to_0  : 1;
  unsigned char sleep_mode      : 1;
  unsigned char apnoe           : 1;
  unsigned char byte1_sync      : 1;

  unsigned char temper_stat     : 2; // 1-0 bits
  unsigned char calib_stat      : 2; // 3-2 bits
  unsigned char compens_not_set : 1; // 4
  unsigned char source_current  : 2; // 6-5 bits
  unsigned char byte2_sync      : 1; // 7

  unsigned char byte3;
  unsigned char byte4;
 // MSB
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
co2_errstat_bits_t;
#pragma pack()

void co2_command_queue_init(void);

void co2_command(int cmd, int nbf, ...);

void co2_process_packet(unsigned char *data, int len);

void co2_undef_state(void);

#endif // __CO2_H

