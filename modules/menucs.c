/*! \file menucs.c
    \brief Menu resources interface.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bedmoni.h"
#include "menucs.h"

#include "datetime.h"
#include "general.h"
#include "patient.h"
#include "settings.h"
#include "alarms.h"
#include "modules.h"
#include "defaultset.h"
#include "networkset.h"
#include "ecgset.h"
#include "spo2set.h"
#include "respset.h"
#include "nibpset.h"
#include "t1t2set.h"
#include "tableset.h"
#include "printset.h"
#include "trendset.h"
#include "shutdown.h"
#include "password.h"
#include "patientnew.h"
#include "stset.h"
#include "arrhset.h"
#include "nibpcal.h"
#include "swupdate.h"
#include "lowbatshutdown.h"
#include "lowbatcharge.h"
#include "langsel.h"
#include "dispset.h"
#include "demoset.h"
#include "leaktst.h"
#include "veriftst.h"
#include "co2set.h"
#include "co2iniset.h"
#include "evlogset.h"
#include "appinfo.h"
#include "adminset.h"

menu_t menus[NUM_MENUS] = 
{
  {MENU_GENERAL,    (unsigned long)menu_general,     menu_general_openproc,     NULL },
  {MENU_DATETIME,   (unsigned long)menu_datetime,    menu_datetime_openproc,    NULL },
  {MENU_PATIENT,    (unsigned long)menu_patient,     menu_patient_openproc,     NULL },
  {MENU_SETTINGS,   (unsigned long)menu_settings,    menu_settings_openproc,    NULL },
  {MENU_ALARMS,     (unsigned long)menu_alarms,      menu_alarms_openproc,      NULL },
  {MENU_MODULES,    (unsigned long)menu_modules,     menu_modules_openproc,     NULL },
  {MENU_DEFAULTSET, (unsigned long)menu_defaultset,  menu_defaultset_openproc,  NULL },
  {MENU_NETWORKSET, (unsigned long)menu_networkset,  menu_networkset_openproc,  NULL },
  {MENU_ECGSET,     (unsigned long)menu_ecgset,      menu_ecgset_openproc,      menu_ecgset_closeproc },
  {MENU_SPO2SET,    (unsigned long)menu_spo2set,     menu_spo2set_openproc,     NULL },
  {MENU_RESPSET,    (unsigned long)menu_respset,     menu_respset_openproc,     NULL },
  {MENU_NIBPSET,    (unsigned long)menu_nibpset,     menu_nibpset_openproc,     NULL },
  {MENU_T1T2SET,    (unsigned long)menu_t1t2set,     menu_t1t2set_openproc,     NULL },
  {MENU_TABLESET,   (unsigned long)menu_tableset,    menu_tableset_openproc,    NULL },
  {MENU_PRINTSET,   (unsigned long)menu_printset,    menu_printset_openproc,    NULL },
  {MENU_TRENDSET,   (unsigned long)menu_trendset,    menu_trendset_openproc,    NULL },
  {MENU_SHUTDOWN,   (unsigned long)menu_shutdown,    menu_shutdown_openproc,    NULL },
  {MENU_PASSWORD,   (unsigned long)menu_password,    menu_password_openproc,    NULL },
  {MENU_PATIENTNEW, (unsigned long)menu_patientnew,  menu_patientnew_openproc,  NULL },
  {MENU_STSET,      (unsigned long)menu_stset,       menu_stset_openproc,       NULL },
  {MENU_ARRHSET,    (unsigned long)menu_arrhset,     menu_arrhset_openproc,     NULL },
  {MENU_NIBPCAL,    (unsigned long)menu_nibpcal,     menu_nibpcal_openproc,     menu_nibpcal_closeproc },
  {MENU_SWUPDATE,   (unsigned long)menu_swupdate,    menu_swupdate_openproc,    NULL },
  {MENU_LOWBATSHUTDOWN, (unsigned long)menu_lowbatshutdown, menu_lowbatshutdown_openproc, NULL },
  {MENU_LOWBATCHARGE,   (unsigned long)menu_lowbat,  menu_lowbat_openproc,      NULL },
  {MENU_LANGSEL,    (unsigned long)menu_langsel,     menu_langsel_openproc,     NULL },
  {MENU_DISPSET,    (unsigned long)menu_dispset,     menu_dispset_openproc,     NULL },
  {MENU_DEMOSET,    (unsigned long)menu_demoset,     menu_demoset_openproc,     menu_demoset_closeproc },
  {MENU_LEAKTEST,   (unsigned long)menu_leaktst,     menu_leaktst_openproc,     menu_leaktst_closeproc },
  {MENU_VERIF,      (unsigned long)menu_veriftst,    menu_veriftst_openproc,    menu_veriftst_closeproc },
  {MENU_CO2SET,     (unsigned long)menu_co2set,      menu_co2set_openproc,      NULL },
  {MENU_CO2INISET,  (unsigned long)menu_co2iniset,   menu_co2iniset_openproc,   NULL },
  {MENU_EVLOGSET,   (unsigned long)menu_evlogset,    menu_evlogset_openproc,    NULL },
  {MENU_APPINFO,    (unsigned long)menu_appinfo,     menu_appinfo_openproc,     NULL },
  {MENU_ADMSET,     (unsigned long)menu_adminset,    menu_adminset_openproc,    NULL },
};

static menucs_item_t * menucs_item_add(menucs_t * pmcs, menucs_item_t * pmcsi)
{
  // printf("%s\n", __FUNCTION__);
  menucs_item_lst_t *ptr, *ptr0;
  if (pmcs->pmil == NULL)
  {
    ptr = (menucs_item_lst_t*) calloc(sizeof(menucs_item_lst_t), 1);
    assert(ptr);
    ptr->next = NULL;
    ptr->item = (menucs_item_t*) calloc(sizeof(menucs_item_t), 1);
    assert(ptr->item);
    memcpy(ptr->item, pmcsi, sizeof(menucs_item_t));
    pmcs->pmil = ptr;
    return ptr->item;
  }
  else
  {
    ptr0 = pmcs->pmil;
    while (ptr0->next != NULL) ptr0 = ptr0->next;
    ptr = (menucs_item_lst_t*) calloc(sizeof(menucs_item_lst_t), 1);
    assert(ptr);
    ptr->next = NULL;
    ptr->item = (menucs_item_t*) calloc(sizeof(menucs_item_t), 1);
    assert(ptr->item);
    memcpy(ptr->item, pmcsi, sizeof(menucs_item_t));
    ptr0->next = ptr;
    return ptr->item;
  }
  return NULL;
}

static void menucs_items_free(menucs_t * pmcs)
{
  menucs_item_lst_t *ptr, *ptr2;

  if (pmcs->pmil == NULL) return;
  ptr = pmcs->pmil;

  do
  {
    ptr2 = ptr->next;
    if (ptr->item) free(ptr->item);
    if (ptr) free(ptr);
    ptr = ptr2;
  } while(ptr != NULL);

  pmcs->pmil = NULL;
}

void menucs_item_proc(char * s, menucs_item_t * pmcsi)
{
  int len;

#ifdef ARM
  len = (s[2]<<8) | s[1];          // total packet len
#else
  len = *((unsigned short*)&s[1]); // total packet len
#endif
  assert(s[0] == MI_BEG && s[len-1]==MI_END);
/*
int i;
for (i=0; i<len; i++)
{
 printf("%02X ", (unsigned char)s[i]);
}
printf("\n");
*/
  pmcsi->type = s[3];

#ifdef ARM
  pmcsi->ids = (s[5]<<8) | s[4];
#else
  pmcsi->ids = *((unsigned short*)&s[4]);
#endif
  pmcsi->ifid = s[6];    // inpfoc id
  pmcsi->ifit = s[7];    // inpfoc item

#ifdef ARM
  pmcsi->x =  (s[9]<<8)  | s[8];
  pmcsi->y =  (s[11]<<8) | s[10];
  pmcsi->cx = (s[13]<<8) | s[12];
  pmcsi->cy = (s[15]<<8) | s[14];
#else
  pmcsi->x  = *((unsigned short*)&s[8]);
  pmcsi->y  = *((unsigned short*)&s[10]);
  pmcsi->cx = *((unsigned short*)&s[12]);
  pmcsi->cy = *((unsigned short*)&s[14]);
#endif

//debug("mi: %d  %d %d %d  %d %d %d %d\n", pmcsi->ids, pmcsi->type, pmcsi->ifid, pmcsi->ifit, pmcsi->x, pmcsi->y, pmcsi->cx, pmcsi->cy);
}

int menucs_load(int id, menucs_t * pmcs)
{
  unsigned char * pb;
  menucs_item_t mcsi;
  int i, it;


//debug("%s +\n", __FUNCTION__);
//printf("len=%d,\n", len);

  if (!pmcs) return -1;

  it = -1;
  for (i=0; i<sizeof(menus)/sizeof(menu_t); i++)
  {
    if (menus[i].id == id)
    {
      it = i;
      break;
    }
  }

  if (it == -1)
  {
    error("id %d not found\n", id);
    return -1;
  }

  pb = (unsigned char*)menus[it].addr;
  pmcs->pmil = NULL;

//debug("ps = %s %d 0x%02X\n", pb, pb, *pb);

  for(i=0;;)
  {
    if (pb[i] == MI_BEG)
    {
      menucs_item_proc((char*)&pb[i], &mcsi);
      menucs_item_add(pmcs, &mcsi);
#ifdef ARM
      i += (pb[i+2] << 8) | pb[i+1];
#else
      i += *((unsigned short*)(&pb[i+1]));
#endif
    }
    else
    {
      // end of resource
      if (pb[i] == MI_EOF)
      {
       // debug("eof\n");
        break;
      }
      // not a menu resource
      error("%s: invalid resource\n", __FUNCTION__);
     // break;
      return -1;
    }
  }

  pmcs->openproc = (i > 4) ? menus[it].openproc : NULL;
  pmcs->closeproc = (i > 4) ? menus[it].closeproc : NULL;

//debug("%s -\n", __FUNCTION__);

  return 0;
}

int menucs_unload(int id, menucs_t * pmcs)
{
  menucs_items_free(pmcs);
  return 0;
}

