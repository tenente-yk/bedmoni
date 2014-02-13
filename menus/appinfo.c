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
#include "dio.h"
#include "password.h"
#include "inpfoc_wnd.h"
#include "appinfo.h"

static void appinfo_none_func(void*);
static void appinfo_upd_func(void*);
static void appinfo_exit_func(void*);

static void read_fw_info(char *s_md5, char *s_flen);

inpfoc_funclist_t inpfoc_appinfo_funclist[APPINFO_NUM_ITEMS+1] = 
{
  { APPINFO_NONE,   appinfo_none_func         },
  { APPINFO_UPD,    appinfo_upd_func          },
  { APPINFO_EXIT,   appinfo_exit_func         },
  { -1       ,      appinfo_none_func         }, // -1 must be last
};

static void appinfo_none_func(void * parg)
{

}

static void appinfo_upd_func(void * parg)
{
  menu_password_onokmenu(MENU_SWUPDATE);
  uframe_command(UFRAME_CHANGE, (void*)MENU_PASSWORD);
}

static void appinfo_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
}

void menu_appinfo_openproc(void)
{
  char s[200], s2[200];

 // uframe_clearbox(10, 200, 180, 20);
  sprintf(s, "%d.%d", HI_VER, LO_VER);
  uframe_printbox(170, 76, 30, 20, s, RGB(0xFF,0xFF,0xFF));

  memset(s, 0, sizeof(s));
  memset(s2, 0, sizeof(s2));
  read_fw_info(s, s2);
  uframe_printbox(10, 176, 190, 20, s+16, RGB(0xFF,0xFF,0xFF));
  s[16] = '\0';
  uframe_printbox(10, 158, 190, 20, s, RGB(0xFF,0xFF,0xFF));

  uframe_printbox(10, 118, 190, 20, s2, RGB(0xFF,0xFF,0xFF));

  inpfoc_set(INPFOC_APPINFO, APPINFO_EXIT);
}

static void read_fw_info(char *s_md5, char *s_flen)
{
  struct stat st;
  FILE * pipe;
  char buf[BUFSIZ];
  char cmd[200];

  if (!s_md5 || !s_flen) return;

#ifdef ARM
  sprintf(cmd, "/usr/local/bin/md5 /usr/local/bin/bmonid");
#else
  sprintf(cmd, "/home/tenente/my_projects/bedmoni/md5 /home/tenente/my_projects/bedmoni/bmonid");
#endif

  st.st_size = 0;
#ifdef ARM
  stat("/usr/local/bin/bmonid", &st);
#else
  stat("/home/tenente/my_projects/bedmoni/bmonid", &st);
#endif
  if (st.st_size > 0)
    sprintf(s_flen, "%d", (int)st.st_size);
  else
    strcpy(s_flen, "?");

  if ((pipe = popen(cmd, "r")) != NULL)
  {
    if (fgets(buf, BUFSIZ, pipe) != NULL)
    {
      if (strchr(buf, ' ')) *(strchr(buf, ' ')) = '\0';
      strcpy(s_md5, buf);
    }
    else
    {
      error("no md5 hash\n");
      *s_md5 = 0;
    }
    pclose(pipe);
  }
}


