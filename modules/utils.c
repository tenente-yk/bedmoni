/*! \file utils.c
    \brief Common functions
 */

#ifdef UNIX
#include <sys/time.h>
#endif
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.h"
#include "dview.h"
#include "grph.h"
#include "bedmoni.h"

unsigned long gettimemillis(void)
{
#ifdef UNIX
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return (tv.tv_sec*1000+tv.tv_usec/1000);
 /*
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC,&ts);
  return (ts.tv_sec*1000+ts.tv_nsec/1000000); // TODO: link with -lrt
 */
#endif
#ifdef WIN32
  return GetTickCount();
#endif
}

void read_hwclock_data(struct tm * ptm)
{
  time_t tt;
  struct tm * pt;

  if (!ptm) return;
  memset(ptm, 0, sizeof(struct tm));

  time(&tt);
  pt = localtime(&tt);
  *ptm = *pt;
#ifdef UNIX
#if 0
  char s[200];
  char dayofweek[10], month[10];
  FILE * fp = popen("hwclock -r", "r");
  if (!fp)
  {
    yfprintf(stderr, "hwclock: error\n");
    return;
  }
  while (!feof(fp))
  {
    fgets(s, sizeof(s), fp);
    yprintf(s);
    sscanf(s, "%s %d %s %d %d:%d:%d ", dayofweek, &ptm->tm_mday, month, &ptm->tm_year, &ptm->tm_hour, &ptm->tm_min, &ptm->tm_sec);
  }
#endif
#endif
}

#ifdef WIN32 //ndef HAVE_STRTOK_R
char * strtok_r(char *s1, const char *s2, char **lasts)
{
   char *ret;

   if (s1 == NULL)
     s1 = *lasts;
   while(*s1 && strchr(s2, *s1))
     ++s1;
   if(*s1 == '\0')
     return NULL;
   ret = s1;
   while(*s1 && !strchr(s2, *s1))
     ++s1;
   if(*s1)
     *s1++ = '\0';
   *lasts = s1;
   return ret;
}
#endif // HAVE_STRTOK_R

void draw_poweroff_message_ex(unsigned short ids, int fd)
{
  char s[200];
  rect_t rc;

  if (ids && ids!=IDS_NONE)
  {
    ids2string(ids, s);
    rc.x0 = 0;
    rc.y0 = 0 + DISPLAY_CY/3;
    rc.x1 = rc.x0 + DISPLAY_CX;
    rc.y1 = DISPLAY_CY/2;

    grph_fillrect(fd, rc.x0+40, rc.y0, rc.x1-rc.x0-40*2, 40, 0x444444);
    grph_setfont(fd, ARIAL_28);
    grph_setfgcolor(fd, RGB(0xff,0xff,0xff));
    grph_setbkcolor(fd, RGB(0x00,0x00,0x00));
    grph_drawtext(fd, s, -1, &rc, DT_CENTER);
  }

  ids2string(IDS_POWERING_OFF, s);
  rc.x0 = 0;
  rc.y0 = 0 + DISPLAY_CY/2;
  rc.x1 = rc.x0 + DISPLAY_CX;
  rc.y1 = rc.y0 + DISPLAY_CY/2;

#if 1
  grph_fillrect(fd, rc.x0+40, rc.y0, rc.x1-rc.x0-40*2, 40, 0x444444);
#else
  // alpha blending by ykiko
  int x,y;
  for (x=rc.x0+40; x<rc.x1-40; x++)
  {
    for (y=rc.y0; y<rc.y0+40; y++)
    {
      unsigned int co;
      unsigned int cf, cb;
      float alf;
      alf = ((float)x-rc.x0+40)/(rc.x1-rc.x0-40*2);
     // alf = fabs(2*alf-1);
      cb = 0x333333;
      cf = 0x437992;
      co =
           (unsigned int) ( ((cb >> 16) & 0xFF) * alf + ((cf >> 16) & 0xFF) * (1-alf) ) << 16 |
           (unsigned int) ( ((cb >>  8) & 0xFF) * alf + ((cf >>  8) & 0xFF) * (1-alf) ) <<  8 |
           (unsigned int) ( ((cb >>  0) & 0xFF) * alf + ((cf >>  0) & 0xFF) * (1-alf) ) <<  0;
      grph_setfgcolor(fd, co);
      grph_line(fd, x, y, x, y);
    }
  }
#endif

  grph_setfont(fd, ARIAL_28);
  grph_setfgcolor(fd, RGB(0xff,0xff,0xff));
  grph_setbkcolor(fd, RGB(0x00,0x00,0x00));
  grph_drawtext(fd, s, -1, &rc, DT_CENTER);
}

void draw_poweroff_message(int fd)
{
  draw_poweroff_message_ex(IDS_NONE, fd);
}

void draw_restart_message(int fd)
{
  char s[200];
  rect_t rc;

  ids2string(IDS_RESTARTING, s);
  rc.x0 = 0;
  rc.y0 = 0 + DISPLAY_CY/2;
  rc.x1 = rc.x0 + DISPLAY_CX;
  rc.y1 = rc.y0 + DISPLAY_CY/2;

#if 1
  grph_fillrect(fd, rc.x0+40, rc.y0, rc.x1-rc.x0-40*2, 40, 0x444444);
#else
  // alpha blending by ykiko
  int x,y;
  for (x=rc.x0+40; x<rc.x1-40; x++)
  {
    for (y=rc.y0; y<rc.y0+40; y++)
    {
      unsigned int co;
      unsigned int cf, cb;
      float alf;
      alf = ((float)x-rc.x0+40)/(rc.x1-rc.x0-40*2);
     // alf = fabs(2*alf-1);
      cb = 0x333333;
      cf = 0x437992;
      co =
           (unsigned int) ( ((cb >> 16) & 0xFF) * alf + ((cf >> 16) & 0xFF) * (1-alf) ) << 16 |
           (unsigned int) ( ((cb >>  8) & 0xFF) * alf + ((cf >>  8) & 0xFF) * (1-alf) ) <<  8 |
           (unsigned int) ( ((cb >>  0) & 0xFF) * alf + ((cf >>  0) & 0xFF) * (1-alf) ) <<  0;
      grph_setfgcolor(fd, co);
      grph_line(fd, x, y, x, y);
    }
  }
#endif

  grph_setfont(fd, ARIAL_28);
  grph_setfgcolor(fd, RGB(0xff,0xff,0xff));
  grph_setbkcolor(fd, RGB(0x00,0x00,0x00));
  grph_drawtext(fd, s, -1, &rc, DT_CENTER);
}

int trtscmp(trts_t * t1, trts_t * t2)
{
  return ((t1->year*12*30*24*60+t1->mon*30*24*60+t1->day*24*60+t1->hour*60+t1->min) - (t2->year*12*30*24*60+t2->mon*30*24*60+t2->day*24*60+t2->hour*60+t2->min));
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}

