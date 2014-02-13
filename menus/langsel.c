/*! \file langsel.c
    \brief Language selection popup frame
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "crbpio.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "lang.h"
#include "iframe.h"
#include "mframe.h"
#include "cframe.h"
#include "langsel.h"

#define LANGSEL_DO_RESTART

static void lang_none_func(void*);
static void lang_rus_func(void*);
static void lang_eng_func(void*);
static void lang_ger_func(void*);
static void lang_fra_func(void*);
static void lang_ita_func(void*);
static void lang_esp_func(void*);
static void lang_ukr_func(void*);
static void lang_exit_func(void*);

inpfoc_funclist_t inpfoc_langsel_funclist[LANG_NUM_ITEMS+1] = 
{
  { LANG_NONE,        lang_none_func         },
  { LANG_RUS,         lang_rus_func          },
  { LANG_ENG,         lang_eng_func          },
  { LANG_GER,         lang_ger_func          },
  { LANG_FRA,         lang_fra_func          },
  { LANG_ITA,         lang_ita_func          },
  { LANG_ESP,         lang_esp_func          },
  { LANG_UKR,         lang_ukr_func          },
  { LANG_EXIT,        lang_exit_func         },
  { -1       ,        lang_none_func         }, // -1 must be last
};

static void lang_none_func(void * parg)
{

}

static void lang_rus_func(void * parg)
{
  lang_set(RUSSIAN);
  lang_reini();

  uframe_command(UFRAME_CHANGE, (void*)MENU_LANGSEL);

#if defined (LANGSEL_DO_RESTART)
  safe_exit(EXIT_RESTART);
#else
  mframe_command(MFRAME_RELOAD, NULL);
  cframe_command(CFRAME_RELOAD, NULL);
  iframe_command(IFRAME_RELOAD, NULL);
  inpfoc_reload(INPFOC_MAIN);
#endif
}

static void lang_eng_func(void * parg)
{
  lang_set(ENGLISH);
  lang_reini();

  uframe_command(UFRAME_CHANGE, (void*)MENU_LANGSEL);

#if defined (LANGSEL_DO_RESTART)
  safe_exit(EXIT_RESTART);
#else
  mframe_command(MFRAME_RELOAD, NULL);
  cframe_command(CFRAME_RELOAD, NULL);
  iframe_command(IFRAME_RELOAD, NULL);
  inpfoc_reload(INPFOC_MAIN);
  #endif
}

static void lang_ger_func(void * parg)
{
  lang_set(GERMAN);
  lang_reini();

  uframe_command(UFRAME_CHANGE, (void*)MENU_LANGSEL);

#if defined (LANGSEL_DO_RESTART)
  safe_exit(EXIT_RESTART);
#else
  mframe_command(MFRAME_RELOAD, NULL);
  cframe_command(CFRAME_RELOAD, NULL);
  iframe_command(IFRAME_RELOAD, NULL);
  inpfoc_reload(INPFOC_MAIN);
#endif
}

static void lang_fra_func(void * parg)
{
  lang_set(FRENCH);
  lang_reini();

  uframe_command(UFRAME_CHANGE, (void*)MENU_LANGSEL);

#if defined (LANGSEL_DO_RESTART)
  safe_exit(EXIT_RESTART);
#else
  mframe_command(MFRAME_RELOAD, NULL);
  cframe_command(CFRAME_RELOAD, NULL);
  iframe_command(IFRAME_RELOAD, NULL);
  inpfoc_reload(INPFOC_MAIN);
#endif
}

static void lang_ita_func(void * parg)
{
  lang_set(ITALIAN);
  lang_reini();

  uframe_command(UFRAME_CHANGE, (void*)MENU_LANGSEL);

#if defined (LANGSEL_DO_RESTART)
  safe_exit(EXIT_RESTART);
#else
  mframe_command(MFRAME_RELOAD, NULL);
  cframe_command(CFRAME_RELOAD, NULL);
  iframe_command(IFRAME_RELOAD, NULL);
  inpfoc_reload(INPFOC_MAIN);
#endif
}

static void lang_esp_func(void * parg)
{
  lang_set(SPANISH);
  lang_reini();

  uframe_command(UFRAME_CHANGE, (void*)MENU_LANGSEL);

#if defined (LANGSEL_DO_RESTART)
  safe_exit(EXIT_RESTART);
#else
  mframe_command(MFRAME_RELOAD, NULL);
  cframe_command(CFRAME_RELOAD, NULL);
  iframe_command(IFRAME_RELOAD, NULL);
  inpfoc_reload(INPFOC_MAIN);
#endif
}

static void lang_ukr_func(void * parg)
{
  lang_set(UKRAINIAN);
  lang_reini();

  uframe_command(UFRAME_CHANGE, (void*)MENU_LANGSEL);

#if defined (LANGSEL_DO_RESTART)
  safe_exit(EXIT_RESTART);
#else
  mframe_command(MFRAME_RELOAD, NULL);
  cframe_command(CFRAME_RELOAD, NULL);
  iframe_command(IFRAME_RELOAD, NULL);
  inpfoc_reload(INPFOC_MAIN);
#endif
}

static void lang_exit_func(void * parg)
{
  // lang_cfg_save not needed since already saved in lang_reini and in environment
  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
}

void menu_langsel_openproc(void)
{
  int it, lang;
  char s[200];

  lang = lang_get();

  switch(lang)
  {
    case RUSSIAN:
      it = LANG_RUS;
      break;
    case ENGLISH:
      it = LANG_ENG;
      break;
    case GERMAN:
      it = LANG_GER;
      break;
    case FRENCH:
      it = LANG_FRA;
      break;
    case ITALIAN:
      it = LANG_ITA;
      break;
    case SPANISH:
      it = LANG_ESP;
      break;
    case UKRAINIAN:
      it = LANG_UKR;
      break;
    default:
      it = LANG_RUS;
  }

  inpfoc_set(INPFOC_LANG, it);

  unsigned char russian_str[] = { 208, 243, 241, 241, 234, 232, 233, 0 };
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_LANG,LANG_RUS), (char*)russian_str);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_LANG,LANG_ENG), "English");
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_LANG,LANG_GER), "Deutsch");
  unsigned char french_str[] = { "Francias" };
  french_str[4] = 166; // c
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_LANG,LANG_FRA), (char*)french_str);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_LANG,LANG_ITA), "Italiano");
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_LANG,LANG_ESP), "Espana");
  unsigned char ukrainian_str[] = { 211, 234, 240, 224, 191, 237, 241, 252, 234, 224, 0 };
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_LANG,LANG_UKR), (char*)ukrainian_str);

/*  inpfoc_wnd_setids(inpfoc_find(INPFOC_LANG,LANG_RUS), IDS_NONE);
  inpfoc_wnd_setids(inpfoc_find(INPFOC_LANG,LANG_ENG), IDS_NONE);
  inpfoc_wnd_setids(inpfoc_find(INPFOC_LANG,LANG_GER), IDS_NONE);
  inpfoc_wnd_setids(inpfoc_find(INPFOC_LANG,LANG_FRA), IDS_NONE);
  inpfoc_wnd_setids(inpfoc_find(INPFOC_LANG,LANG_ITA), IDS_NONE);
  inpfoc_wnd_setids(inpfoc_find(INPFOC_LANG,LANG_ESP), IDS_NONE);
  inpfoc_wnd_setids(inpfoc_find(INPFOC_LANG,LANG_UKR), IDS_NONE);
*/
  sprintf(s, "%s/%s/%s", USBDISK_PATH, "fonts", lang_fontdir[RUSSIAN]);
  it = access(s, R_OK);
  if (it != 0)
  {
    sprintf(s, "%s/%s/%s", "/usr/local", "fonts", lang_fontdir[RUSSIAN]);
    it = access(s, R_OK);
  }
  if (it != 0) inpfoc_disable(INPFOC_LANG, LANG_RUS);

  sprintf(s, "%s/%s/%s", USBDISK_PATH, "fonts", lang_fontdir[ENGLISH]);
  it = access(s, R_OK);
  if (it != 0)
  {
    sprintf(s, "%s/%s/%s", "/usr/local", "fonts", lang_fontdir[ENGLISH]);
    it = access(s, R_OK);
  }
  if (it != 0) inpfoc_disable(INPFOC_LANG, LANG_ENG);
  sprintf(s, "%s/%s/%s", USBDISK_PATH, "fonts", lang_fontdir[GERMAN]);
  it = access(s, R_OK);
  if (it != 0)
  {
    sprintf(s, "%s/%s/%s", "/usr/local", "fonts", lang_fontdir[GERMAN]);
    it = access(s, R_OK);
  }
  if (it != 0) inpfoc_disable(INPFOC_LANG, LANG_GER);
  sprintf(s, "%s/%s/%s", USBDISK_PATH, "fonts", lang_fontdir[FRENCH]);
  it = access(s, R_OK);
  if (it != 0)
  {
    sprintf(s, "%s/%s/%s", "/usr/local", "fonts", lang_fontdir[FRENCH]);
    it = access(s, R_OK);
  }
  if (it != 0) inpfoc_disable(INPFOC_LANG, LANG_FRA);
  sprintf(s, "%s/%s/%s", USBDISK_PATH, "fonts", lang_fontdir[ITALIAN]);
  it = access(s, R_OK);
  if (it != 0)
  {
    sprintf(s, "%s/%s/%s", "/usr/local", "fonts", lang_fontdir[ITALIAN]);
    it = access(s, R_OK);
  }
  if (it != 0) inpfoc_disable(INPFOC_LANG, LANG_ITA);
  sprintf(s, "%s/%s/%s", USBDISK_PATH, "fonts", lang_fontdir[SPANISH]);
  it = access(s, R_OK);
  if (it != 0)
  {
    sprintf(s, "%s/%s/%s", "/usr/local", "fonts", lang_fontdir[SPANISH]);
    it = access(s, R_OK);
  }
  if (it != 0) inpfoc_disable(INPFOC_LANG, LANG_ESP);
  sprintf(s, "%s/%s/%s", USBDISK_PATH, "fonts", lang_fontdir[UKRAINIAN]);
  it = access(s, R_OK);
  if (it != 0)
  {
    sprintf(s, "%s/%s/%s", "/usr/local", "fonts", lang_fontdir[UKRAINIAN]);
    it = access(s, R_OK);
  }
  if (it != 0) inpfoc_disable(INPFOC_LANG, LANG_UKR);
}

