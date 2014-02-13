#ifndef __PAT_H
#define __PAT_H

//#define NEONATE_FEATURE

enum
{
  ADULT = 0,
  CHILD,
#if defined (NEONATE_FEATURE)
  NEONATE,
#endif
  NUM_PAT_CAT,
};

#pragma pack(1)
typedef struct
{
  int type;
  int bedno;
  int cardno;
  int w;
  int h;
} pat_pars_t;
#pragma pack()

typedef pat_pars_t pat_cfg_t;

void pat_get(pat_pars_t * pp);
void pat_set(pat_pars_t * pp);
void pat_ini(pat_pars_t * pp);
void pat_ini_def(pat_pars_t * pp);
void pat_deini(void);

extern unsigned short pat_cat_ids[NUM_PAT_CAT];

#endif // __PAT_H
