#ifndef __ECGCALC_H
#define __ECGCALC_H

#include "fifo.h"

// ecg calc settings' command range 0x50 - 0x5f
enum
{
  ECS_JPOINT_SHIFT = 0x50,   // 0x50
  ECS_ARRH,                  // 0x51
  ECS_RESTART_EP,            // 0x52
  ECS_PM_NORESP_MS,          // 0x53
  ECS_RESP_APNOE_S,          // 0x54
  ECS_RESPCALC_RESET,        // 0x55
};

#ifdef __cplusplus
extern "C" {
#endif

typedef int ecg_t;

//#pragma pack(push, 1)
#pragma pack (1)

typedef struct
{
  unsigned short hr;
  short          st[NUM_ECG];
  int            num_ecg_chns;
} ecg_pars_t;
//} __attribute__ ((__packed__)) ecg_pars_t;

//#pragma pack (pop)
#pragma pack ()

#define FD_ECG                     500
#define SEARCH_INTERVAL_ECG        3*FD_ECG
#define MAX_BUF_SIZE_ECG           (2*SEARCH_INTERVAL_ECG)
#define ECGCALC_UNDEF_VALUE        22222

#ifdef __cplusplus
}
#endif

#endif // __ECGCALC_H
