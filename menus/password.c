#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "stconf.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "password.h"

static void password_none_func(void*);
static void password_u1_func(void*);
static void password_u2_func(void*);
static void password_u3_func(void*);
static void password_u4_func(void*);
static void password_ok_func(void*);
static void password_exit_func(void*);

static int menuid = -1;
static unsigned int password = 0x00000000;

inpfoc_funclist_t inpfoc_password_funclist[PASSWORD_NUM_ITEMS+1] = 
{
  { PASSWORD_NONE,     password_none_func        },
  { PASSWORD_U1,       password_u1_func          },
  { PASSWORD_U2,       password_u2_func          },
  { PASSWORD_U3,       password_u3_func          },
  { PASSWORD_U4,       password_u4_func          },
  { PASSWORD_OK,       password_ok_func          },
  { PASSWORD_EXIT,     password_exit_func        },
  { -1       ,         password_none_func        }, // -1 must be last
};

static void password_none_func(void * parg)
{

}

static void password_ok_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  int v;
  unsigned int password_inp;
  char s[200];

  password_inp = 0x00000000;

  pit = inpfoc_find(INPFOC_PASSWORD, PASSWORD_U1);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &v);
  password_inp |= v << 24;

  pit = inpfoc_find(INPFOC_PASSWORD, PASSWORD_U2);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &v);
  password_inp |= v << 16;

  pit = inpfoc_find(INPFOC_PASSWORD, PASSWORD_U3);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &v);
  password_inp |= v << 8;

  pit = inpfoc_find(INPFOC_PASSWORD, PASSWORD_U4);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &v);
  password_inp |= v << 0;

  if (password_inp == menu_password_getpass())
  {
    uframe_command(UFRAME_CHANGE, (void*) menuid);
  }
  else
  {
    stext_t stext;
    memset(&stext, 0, sizeof(stext_t));
    stext.x = 30;
    stext.y = 90;
    stext.color = RGB(0x77,0,0); //UFRAME_STATICTEXT_COLOR;
    ids2string(IDS_INVALID_PASSWORD, s);
    strcpy(stext.s, s);
    uframe_command(UFRAME_SETSTEXT, (void*)&stext);
    memset(&stext, 0, sizeof(stext_t));
    stext.x = 20;
    stext.y = 108;
    stext.color = RGB(0x77,0,0);
    ids2string(IDS_TRY_AGAIN, s);
    strcpy(stext.s, s);
    uframe_command(UFRAME_SETSTEXT, (void*)&stext);
    inpfoc_set(INPFOC_PASSWORD, PASSWORD_U1);
  }
}

static void password_u1_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int v;
  char s[10];

  pit = inpfoc_find(INPFOC_PASSWORD, PASSWORD_U1);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &v);
  if (pcmd->delta > 0)
    if (++v > 99) v = 0;
  if (pcmd->delta < 0)
    if (--v < 0) v = 99;
  snprintf(s, sizeof(s), "%02d", v);

  inpfoc_wnd_setcaption(pit, s);
}

static void password_u2_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int v;
  char s[10];

  pit = inpfoc_find(INPFOC_PASSWORD, PASSWORD_U2);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &v);
  if (pcmd->delta > 0)
    if (++v > 99) v = 0;
  if (pcmd->delta < 0)
    if (--v < 0) v = 99;
  snprintf(s, sizeof(s), "%02d", v);

  inpfoc_wnd_setcaption(pit, s);
}

static void password_u3_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int v;
  char s[10];

  pit = inpfoc_find(INPFOC_PASSWORD, PASSWORD_U3);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &v);
  if (pcmd->delta > 0)
    if (++v > 99) v = 0;
  if (pcmd->delta < 0)
    if (--v < 0) v = 99;
  snprintf(s, sizeof(s), "%02d", v);

  inpfoc_wnd_setcaption(pit, s);
}

static void password_u4_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_wnd_t  *pifw;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  int v;
  char s[10];

  pit = inpfoc_find(INPFOC_PASSWORD, PASSWORD_U4);
  pifw = (inpfoc_wnd_t*) pit->parg;
  sscanf(pifw->caption, "%02d", &v);
  if (pcmd->delta > 0)
    if (++v > 99) v = 0;
  if (pcmd->delta < 0)
    if (--v < 0) v = 99;
  snprintf(s, sizeof(s), "%02d", v);

  inpfoc_wnd_setcaption(pit, s);
}

static void password_exit_func(void * parg)
{
  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
}

void menu_password_openproc(void)
{
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PASSWORD, PASSWORD_U1), "00");
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PASSWORD, PASSWORD_U2), "00");
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PASSWORD, PASSWORD_U3), "00");
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_PASSWORD, PASSWORD_U4), "00");
  inpfoc_set(INPFOC_PASSWORD, PASSWORD_U1);
}

void menu_password_onokmenu(int id)
{
  menuid = id;
}

unsigned int menu_password_getpass(void)
{
  FILE * pipe;
  char buf[BUFSIZ];
  char cmd[100], u[4], *s;

  strcpy(cmd, "/usr/local/bin/fw_printenv bedmoni_pw");

  if ((pipe = popen(cmd, "r")) != NULL)
  {
    if (fgets(buf, BUFSIZ, pipe) != NULL)
    {
      memset(u, 0, sizeof(u));
      s = buf;
      while (*s != '=' && *s) s++;
      if (*s) s++;
      u[0] = atoi(s);
      while (*s != ':' && *s) s++;
      if (*s) s++;
      u[1] = atoi(s);
      while (*s != ':' && *s) s++;
      if (*s) s++;
      u[2] = atoi(s);
      while (*s != ':' && *s) s++;
      if (*s) s++;
      u[3] = atoi(s);
      password = (u[0] << 24) | (u[1] << 16) | (u[2] << 8) | (u[3] << 0);
      debug("password = %d %d %d %d\n", (int)u[0], (int)u[1], (int)u[2], (int)u[3]);
    }
    else
    {
      debug("no password in env, return default\n");
      password = 0x00000000;
    }
    pclose(pipe);
  }

  return password;
}

void menu_password_setpass(unsigned int pw)
{
  password = pw;
}


