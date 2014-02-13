#ifndef __DISP_H
#define __DISP_H

#pragma pack(1)
typedef struct
{
  unsigned char brightness; // 1-100
}
dispset_t;
#pragma pack()

// implementation in menus/dispset.c
void disp_ini(void);

void disp_deini(void);

void disp_ini_def(void);

void disp_get(dispset_t * pds);

void disp_set(dispset_t * pds);

void disp_cfg_save(void);

#endif // __DISP_H
