/*! \file lang.c
    \brief Language interface
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
//#include <pthread.h>
#include "bedmoni.h"
#include "stconf.h"
#include "lang.h"

#include "inpfoc.h"

//static pthread_mutex_t fids_mutex = PTHREAD_MUTEX_INITIALIZER;

static FILE *fids = 0;
static int current_language;
static char langfname[NUM_LANGUAGES][100] = 
{
  "russian",   // RUSSIAN
  "english",   // ENGLISH
  "german",    // GERMAN
  "french",    // FRENCH
  "italian",   // ITALIAN
  "spanish",   // SPANISH
  "ukrainian", // UKRAINIAN
};

char lang_fontdir[NUM_LANGUAGES][4] = 
{
  { "rus" },
  { "eng" },
  { "ger" },
  { "fra" },
  { "ita" },
  { "esp" },
  { "ukr" },
};

void lang_ini(void)
{
  int lang;
  char s[200];

  // default - RUSSIAN
  lang_set(RUSSIAN);

  if ( stconf_read(STCONF_LANG, &lang, sizeof(lang)) > 0 )
  {
    current_language = lang;
  }

//printf("ALWAYS ENGLISH!!!\n");
//  current_language = ENGLISH;

  lang_set(current_language);

  sprintf(s, "%s/%s/%s.lang", USBDISK_PATH, DATA_DIR, langfname[current_language]);
  fids = fopen(s, "rb");
  if (!fids)
  {
    sprintf(s, "/usr/local/%s/%s.lang", DATA_DIR, langfname[current_language]);
    fids = fopen(s, "rb");
  }
}

void lang_deini(void)
{
 // lang_cfg_save();
  if (fids) fclose(fids);
  fids = NULL;
}

void lang_reini(void)
{
  lang_cfg_save();
  lang_deini();
  lang_ini();
#if 0
  char s[200];
  if (fids) fclose(fids);
  sprintf(s, "%s/%s/%s.lang", USBDISK_PATH, DATA_DIR, langfname[current_language]);
  fids = fopen(s, "rb");
  if (!fids)
  {
    sprintf(s, "/usr/local/%s/%s.lang", DATA_DIR, langfname[current_language]);
    fids = fopen(s, "rb");
  }
#endif
}

void lang_set(unsigned char lang)
{
  if (lang >= NUM_LANGUAGES) return;
  current_language = lang;
//debug("for a while\n");
//inpfoc_invalidate(INPFOC_MAIN);
}

unsigned char lang_get(void)
{
  return current_language;
}

void ids2string(unsigned short ids, char * s)
{
  unsigned short w;

  if (!s) return;

  if (!fids)
  {
    strcpy(s, "---");
    return;
  }

 // pthread_mutex_lock(&fids_mutex);

  fseek(fids, ids*128, SEEK_SET);
  fread(&w, 1, 2, fids);
  if (w == ids)
    fread(s, 1, 128-2, fids);
  else
    strcpy(s, "---");

 // pthread_mutex_unlock(&fids_mutex);
}

void lang_cfg_save(void)
{
  if ( stconf_write(STCONF_LANG, &current_language, sizeof(current_language)) > 0 );
#if 1
#ifdef ARM

  // store current language in u-boot env
  char cmd[200];
  sprintf(cmd, "fw_setenv bedmoni_lang %s > /dev/null", langfname[current_language]);
#if 1
  system(cmd);
#else // causes artifacts on the screen
  int pid1;
  pid1 = fork();
  if (pid1 < 0)
  {
    error("lang_cfg_save: unable to fork\n");
    system(cmd);
    return;
  }
  if (pid1 == 0)
  {
   // sprintf(cmd, "fw_setenv bedmoni_lang %s > /dev/null", langfname[current_language]);
    execl("/usr/local/bin/fw_setenv", "fw_setenv", "bedmoni_lang", langfname[current_language], NULL);
    exit(0);
  }
#endif

#endif
#endif
}
