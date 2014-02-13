#ifndef __LANG_H
#define __LANG_H

enum
{
  RUSSIAN,
  ENGLISH,
  GERMAN,
  FRENCH,
  ITALIAN,
  SPANISH,
  UKRAINIAN,
  NUM_LANGUAGES,
};

void lang_ini(void);

void lang_deini(void);

void lang_set(unsigned char lang);

unsigned char lang_get(void);

void ids2string(unsigned short ids, char * s);

void lang_cfg_save(void);

void lang_reini(void);

extern char lang_fontdir[NUM_LANGUAGES][4];

#endif // __LANG_H
