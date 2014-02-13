#ifndef __UTILS_H
#define __UTILS_H

#include <time.h>

typedef struct
{
  unsigned int hour : 5;
  unsigned int min  : 6;
  unsigned int day  : 6;
  unsigned int mon  : 4;
  unsigned int year : 8;
  unsigned int beg  : 1;
  unsigned int unused : 2;
} trts_t;

/*! \fn unsigned long gettimemillis(void)
 *  \brief The gettimemillis function retrieves the number of milliseconds that have elapsed since the system was started.
 *  \return The return value is the number of milliseconds that have elapsed since the system was started.
 */
unsigned long gettimemillis(void);

void read_hwclock_data(struct tm * ptm);

void ids2string(unsigned short ids, char * s); // implemented in lang.c

void draw_poweroff_message(int fd);

void draw_poweroff_message_ex(unsigned short ids, int fd);

void draw_restart_message(int fd);

int trtscmp(trts_t * tr1, trts_t * tr2);

int kbhit(void);

#endif // __UTILS_H
