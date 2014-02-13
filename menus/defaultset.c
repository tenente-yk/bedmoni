/*! \file defaultset.c
    \brief Popup frame with default settings dialog window
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "bedmoni.h"
#include "mframe.h"
#include "stconf.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "patient.h"
#include "networkset.h"
#include "tframe.h"
#include "lframe.h"
#include "lang.h"
#include "tr.h"
#include "defaultset.h"

static void defset_none_func(void*);
static void defset_yes_func(void*);
static void defset_no_func(void*);

inpfoc_funclist_t inpfoc_defset_funclist[DEFSET_NUM_ITEMS+1] = 
{
  { DEFSET_NONE,       defset_none_func         },
  { DEFSET_YES,        defset_yes_func          },
  { DEFSET_NO,         defset_no_func           },
  { -1       ,         defset_none_func         }, // -1 must be last
};

static void defset_none_func(void * parg)
{

}

static void defset_yes_func(void * parg)
{
  char s[200];

  debug("Set default settings\n");

  tr_reset();

  stconf_deinit();
  sprintf(s, "%s/%s/%s", USBDISK_PATH, DATA_DIR, CONFIG_FNAME);
  if ( unlink(s) != 0)
  {
    error("config file has not been removed\n");
  }
#ifdef ARM
  sprintf(s, "%s/%s", USBDISK_PATH, "sno.txt");
  if ( unlink(s) != 0)
  {
    error("serial_no file has not been removed\n");
  }
#endif
  lang_ini();
  //stconf_init();
  // one more time stconf_deinit will be called in deinit functions (bedmoni.c)

#if 1 // bmonid fix
  // need to create 1.tar.gz in /mnt/usbdisk containing bin/bedmoni
  // prevent creating bin file instead bin directory in /mnt/usbdisk
  // by bmonid after reboot
#endif

#if 0
  pat_ini_def(NULL);
  network_ini_def(NULL);
 // unit_ini_def(FALSE);
  unit_ini_def(TRUE);
  alarm_ini_def();
  tframe_ini_def();
  lframe_ini_def();

  stconf_init();

  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
#endif
  safe_exit(EXIT_UPDATED);
}

static void defset_no_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
}

void menu_defaultset_openproc(void)
{
  inpfoc_set(INPFOC_DEFSET, DEFSET_NO);
}
