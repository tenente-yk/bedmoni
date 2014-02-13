#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "bedmoni.h"
#include "stconf.h"
#include "iframe.h"
#include "pat.h"

unsigned short pat_cat_ids[NUM_PAT_CAT] = 
{
  IDS_ADULT,
  IDS_CHILD,
#if defined (NEONATE_FEATURE)
  IDS_NEONATE,
#endif
};

static pat_pars_t pat_pars;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void pat_get(pat_pars_t * pp)
{
  pthread_mutex_lock(&mutex);
  if (pp) *pp = pat_pars;
  pthread_mutex_unlock(&mutex);
}

void pat_set(pat_pars_t * pp)
{
  pthread_mutex_lock(&mutex);
  if (pp) memcpy(&pat_pars, pp, sizeof(pat_pars_t));
  pthread_mutex_unlock(&mutex);
}

void pat_ini(pat_pars_t * pp)
{
  pat_pars_t lpp;
  pat_cfg_t pat_cfg;

  pat_ini_def(pp);

  memset(&lpp, 0, sizeof(pat_pars_t));
  if ( stconf_read(STCONF_PATIENT, &pat_cfg, sizeof(pat_cfg_t)) > 0 )
  {
    assert( sizeof(pat_cfg_t) == sizeof(pat_pars_t) );
    memcpy(&lpp, &pat_cfg, sizeof(pat_pars_t));
    pat_set(&lpp);
  }
  else
    return;

  if (pp)
    memcpy(pp, &lpp, sizeof(pat_pars_t));

  iframe_command(IFRAME_RELOAD, NULL);
}

void pat_deini(void)
{
  pat_pars_t lpp;
  pat_cfg_t pat_cfg;

  memset(&lpp, 0, sizeof(pat_pars_t));
  pat_get(&lpp);

  assert( sizeof(pat_pars_t) == sizeof(pat_cfg_t) );
  memcpy(&pat_cfg, &lpp, sizeof(pat_cfg_t));
  if ( stconf_write(STCONF_PATIENT, &pat_cfg, sizeof(pat_cfg_t)) > 0 );
}

void pat_ini_def(pat_pars_t * pp)
{
  pat_pars_t lpp;

  lpp.type    = ADULT;
  lpp.bedno   = 1; // 1 - 999
  lpp.cardno  = 1; // 1 - 99999
  lpp.w       = 68;
  lpp.h       = 184;

  pat_set(&lpp);

  if (pp)
    memcpy(pp, &lpp, sizeof(pat_pars_t));

  iframe_command(IFRAME_RELOAD, NULL);
}
