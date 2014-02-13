#ifndef __TPRN_H
#define __TPRN_H

#ifndef ARM
#define TPRN_DISABLED
#endif
//#define TPRN_DEBUG
//#define TPRN_TRANSPARENT_INKS

#ifndef ARM
#undef TPRN_TRANSPARENT_INKS
#endif

#include "dview.h"

#define TPRN_ID_DATA           0x07
#define TPRN_ID_SETUP          0x08
#define TPRN_ID_STAT           0x09

#define TPRN_PACKET_SIZE       (48+4)
#define TPRN_PAPER_DPM         8
#define TPRN_PAPER_DW          384
#define TPRN_MAX_SEC_TO_PRINT  17
#define TPRN_MAX_FIFO_SIZE     (TPRN_MAX_SEC_TO_PRINT*TPRN_PACKET_SIZE*TPRN_PAPER_DW*50 + 1000*TPRN_PACKET_SIZE) // 17 sec from memory, 50 mm/sec and header
#define TPRN_STORED_DATA_SIZE  (6*1000)    // 6 sec, adc freq 1000 Hz

#define TPRNIN_BUF_SIZE        100
#define TPRN_MSG_SIZE          8
#define TPRN_TS_PERIOD_MS      100

#define TPRN_WIDTH_1           0x1
#define TPRN_WIDTH_2           0x2
#define TPRN_WIDTH_3           0x3
#define TPRN_BRIGHTNESS_MASK   0xf
#define TPRN_BRIGHTNESS_0      0x0
#define TPRN_BRIGHTNESS_10     0x1
#define TPRN_BRIGHTNESS_20     0x2
#define TPRN_BRIGHTNESS_30     0x3
#define TPRN_BRIGHTNESS_40     0x4
#define TPRN_BRIGHTNESS_50     0x5
#define TPRN_BRIGHTNESS_60     0x6
#define TPRN_BRIGHTNESS_70     0x7
#define TPRN_BRIGHTNESS_80     0x8
#define TPRN_BRIGHTNESS_90     0x9
#define TPRN_BRIGHTNESS_100    0xA

#define TPRN_UNDEF_CURVE       0xFF
#define TPRN_NUM_CURVES        2
#ifdef ARM
#define TPRNIO_COMMPORT        "/dev/ttyUSB0"
#else
#define TPRNIO_COMMPORT        "/dev/ttyUSB1"
#endif

#define TPRN_START             '<'
#define TPRN_END               '>'

enum
{
  TPRN_AMP_2_5 = 0,
 // TPRN_AMP_AUTO = TPRN_AMP_2_5,
  TPRN_AMP_X1 = TPRN_AMP_2_5,
  TPRN_AMP_5,
  TPRN_AMP_X2 = TPRN_AMP_5,
  TPRN_AMP_10,
  TPRN_AMP_X4 = TPRN_AMP_10,
  TPRN_AMP_20,
  TPRN_AMP_X8 = TPRN_AMP_20,
  TPRN_AMP_40,
  TPRN_AMP_X16 = TPRN_AMP_40,
  MAXNUM_TPRN_AMP,
};

enum
{
  TPRN_SPD_12_5 = 0,
  TPRN_SPD_25,
  TPRN_SPD_50,
  MAXNUM_TPRN_SPD,
};

#pragma pack(1)

typedef struct
{
  unsigned short grid        : 1;
  unsigned short val         : 1;
  unsigned short width       : 2;
  unsigned short started     : 1;
  unsigned short brightness  : 4;
  unsigned short __unused    : 7;
}
tprn_prnmask_t;

typedef struct
{
  unsigned char   n;       // num curves
  unsigned char   c[2];    // curves' type
  unsigned char   amp[2];  // curves' amp
  unsigned char   spd;
  tprn_prnmask_t  prnmask;
  unsigned char   delay1;
  unsigned char   delay2;
  unsigned char   pos;
  unsigned char   unused[1];
}
tprn_cfg_t;

typedef struct
{
  int           data[TPRN_STORED_DATA_SIZE];
  unsigned int  dt[TPRN_STORED_DATA_SIZE];
  unsigned int  num[TPRN_STORED_DATA_SIZE];
  unsigned int  wdptr;
  unsigned int  wiptr;
 // unsigned int  rdptr;
 // unsigned int  riptr;
} tprn_stored_data_t;

#pragma pack()

void tprn_init(void);

void tprn_ini_def(void);

void tprn_deinit(void);

int tprn_cfg_get(tprn_cfg_t * ptr);

int tprn_cfg_set(tprn_cfg_t * ptr);

void tprn_push_data(int no, int * buf, int nPoints, unsigned long dwTimeMS);

void tprn_reset_grph(void);

void tprn_start(void);

void tprn_stop(void);

void tprn_cfg_save(void);

void tprn_command(int id, unsigned char data0, unsigned char data1, unsigned char data2, unsigned char data3);

void tprn_stepit(void);

extern float tprn_amp[MAXNUM_TPRN_AMP];
extern float tprn_amp_mul[NUM_VIEWS];
extern float tprn_spd[MAXNUM_TPRN_SPD];

#endif // __TPRN_H
