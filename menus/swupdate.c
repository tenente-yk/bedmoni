/*! \file swupdate.c
    \brief Software update popup frame
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "bedmoni.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "networkset.h"
#include "dio.h"
#include "elog.h"
#include "upd.h"
#include "swupdate.h"

static void swupd_none_func(void*);
static void swupd_ips1_func(void*);
static void swupd_ips2_func(void*);
static void swupd_ips3_func(void*);
static void swupd_ips4_func(void*);
static void swupd_ips_ok_func(void*);
static void swupd_exit_func(void*);

static int cancel_update = 0;

static int fd = 0;

inpfoc_funclist_t inpfoc_swupdate_funclist[SWUPD_NUM_ITEMS+1] = 
{
  { SWUPD_NONE,     swupd_none_func         },
  { SWUPD_IPS1,     swupd_ips1_func         },
  { SWUPD_IPS2,     swupd_ips2_func         },
  { SWUPD_IPS3,     swupd_ips3_func         },
  { SWUPD_IPS4,     swupd_ips4_func         },
  { SWUPD_IPS_OK,   swupd_ips_ok_func       },
  { SWUPD_EXIT,     swupd_exit_func         },
  { -1       ,      swupd_none_func         }, // -1 must be last
};

static void swupd_none_func(void * parg)
{

}

static void swupd_ips1_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  networkset_t nws;
  int v;
  char s[30];

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);
  v = (nws.serverip & 0xff000000) >> 24;

  v += pcmd->delta;
  pit = inpfoc_find(INPFOC_SWUPD, SWUPD_IPS1);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.serverip &= 0x00ffffff;
  nws.serverip |= (v << 24);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);
}

static void swupd_ips2_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  networkset_t nws;
  int v;
  char s[30];

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);
  v = (nws.serverip & 0x00ff0000) >> 16;

  v += pcmd->delta;
  pit = inpfoc_find(INPFOC_SWUPD, SWUPD_IPS2);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.serverip &= 0xff00ffff;
  nws.serverip |= (v << 16);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);
}

static void swupd_ips3_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  networkset_t nws;
  int v;
  char s[30];

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);
  v = (nws.serverip & 0x0000ff00) >> 8;

  v += pcmd->delta;
  pit = inpfoc_find(INPFOC_SWUPD, SWUPD_IPS3);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.serverip &= 0xffff00ff;
  nws.serverip |= (v << 8);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);
}

static void swupd_ips4_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  networkset_t nws;
  int v;
  char s[30];

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);
  v = (nws.serverip & 0x000000ff) >> 0;

  v += pcmd->delta;
  pit = inpfoc_find(INPFOC_SWUPD, SWUPD_IPS4);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.serverip &= 0xffffff00;
  nws.serverip |= (v << 0);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);
}

static void swupd_ips_ok_func(void * parg)
{
  char s[200];

  sprintf(s, "%s/%s", USBDISK_PATH, "1.tar.gz");
  unlink(s);

  if (fd == 0)
  {
    int portno;
    networkset_t nws;

    memset(&nws, 0, sizeof(networkset_t));
    network_get(&nws);

    snprintf(s, sizeof(s), "%d.%d.%d.%d", (nws.serverip & 0xff000000) >> 24, (nws.serverip & 0x00ff0000) >> 16, (nws.serverip & 0x0000ff00) >> 8, (nws.serverip & 0x000000ff) >> 0);
    portno = nws.portno;

    fd = upd_client_open(s, portno);
    if (fd == 0)
    {
      debug("unable to create connection\n");
      upd_client_close(fd);
     // uframe_command(UFRAME_DESTROY, NULL);
      ids2string(IDS_UNABLE_CONNECT, s);
      uframe_printbox(20, 40, 160, 20, s, RGB(0xAA,0x00,0x00));
      return;
    }
    else
    {
      elog_add(EL_UPDATE, 0, 0);

      uframe_command(UFRAME_UPDATEMENU, (void*) MENU_SWUPDATE);
    }
  }
}

static void swupd_exit_func(void * parg)
{
  if (fd != 0)
  {
    cancel_update = 1;
    uframe_command(UFRAME_UPDATEMENU, (void*) MENU_SWUPDATE);
  }
  else
  {
   // uframe_command(UFRAME_DESTROY, NULL);
    uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
  }
}

void menu_swupdate_openproc(void)
{
  char s[200], s2[200];
  networkset_t nws;
  int pcnts_completed;

  if (fd != 0)
  {
    upd_stepit(fd);

    if (cancel_update)
    {
      debug("cancel update...\n");
      cancel_update = 0;
      upd_client_close(fd);
      fd = 0;
     // uframe_command(UFRAME_DESTROY, NULL);
      uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
      return;
    }

    if (upd_get_state() == UPD_STATE_FINISHED)
    {
      debug("update complete\n");

      upd_client_close(fd);
      fd = 0;
      ids2string(IDS_COMPLETED, s2);
      sprintf(s, "%s: %d%%", s2, 100);
      uframe_printbox(40, 40, -1, -1, s, RGB(0x00,0x99,0x00));
     // uframe_command(UFRAME_DESTROY, NULL);

      // store default language to environment
      lang_set(RUSSIAN);
      lang_reini();

#if 1
      safe_exit(EXIT_UPDATED);
#endif
      return;
    }
    else
    {
      pcnts_completed = upd_get_progress();
     // debug("%s: %d%%\n", "completed ", pcnts_completed);
      ids2string(IDS_COMPLETED, s2);
      sprintf(s, "%s: %d%%", s2, pcnts_completed+1); // 99% -> 100%
      uframe_clearbox(10, 40, 180, 20);
      uframe_printbox(40, 40, -1, -1, s, RGB(0x00,0x99,0x00));
      uframe_command(UFRAME_UPDATEMENU, (void*) MENU_SWUPDATE);
    }
  }
  else
  {
    memset(&nws, 0, sizeof(networkset_t));
    network_get(&nws);

    snprintf(s, sizeof(s), "%d", (nws.serverip & 0xff000000) >> 24);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_SWUPD, SWUPD_IPS1), s);
    snprintf(s, sizeof(s), "%d", (nws.serverip & 0x00ff0000) >> 16);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_SWUPD, SWUPD_IPS2), s);
    snprintf(s, sizeof(s), "%d", (nws.serverip & 0x0000ff00) >> 8);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_SWUPD, SWUPD_IPS3), s);
    snprintf(s, sizeof(s), "%d", (nws.serverip & 0x000000ff) >> 0);
    inpfoc_wnd_setcaption(inpfoc_find(INPFOC_SWUPD, SWUPD_IPS4), s);
    inpfoc_set(INPFOC_SWUPD, SWUPD_EXIT);
  }
}
