#ifndef __STCONF_H
#define __STCONF_H

#define CONFIG_FNAME  "stconf.cfg"

enum
{
  STCONF_NONE,      // 0
  STCONF_CFRAME,    // 1
  STCONF_MFRAME,    // 2
  STCONF_NETWORK,   // 3
  STCONF_TFRAME,    // 4
  STCONF_LFRAME,    // 5
  STCONF_PATIENT,   // 6
  STCONF_UNITS,     // 7
  STCONF_ALARM,     // 8
  STCONF_TPRN,      // 9
  STCONF_DEMO,      // 10
  STCONF_LANG,      // 11
  STCONF_DISP,      // 12
  STCONF_EFRAME,    // 13
  STCONF_14,
  STCONF_15,
  STCONF_16,
  STCONF_17,
  STCONF_18,
  STCONF_19,
  STCONF_20,
  STCONF_21,
  STCONF_22,
  STCONF_23,
  STCONF_24,
  STCONF_25,
  STCONF_26,
  STCONF_27,
  STCONF_28,
  STCONF_29,
  STCONF_30,
  STCONF_31,
  STCONF_32,
  STCONF_NUM_ID,
};

typedef struct
{
  unsigned int id;
  unsigned int offs;
  unsigned int len;
  unsigned int num_id;
} stconf_hdr_t;

int  stconf_init(void);

void stconf_deinit(void);

int  stconf_read(int id, void * buffer, int len);

int  stconf_write(int id, const void * buffer, int len);

#endif // __STCONF_H
