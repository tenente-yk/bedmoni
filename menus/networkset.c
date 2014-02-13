#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "bedmoni.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "csio.h"
#include "stconf.h"
#include "networkset.h"

static void netset_none_func(void*);
static void netset_ipm1_func(void*);
static void netset_ipm2_func(void*);
static void netset_ipm3_func(void*);
static void netset_ipm4_func(void*);
static void netset_ips1_func(void*);
static void netset_ips2_func(void*);
static void netset_ips3_func(void*);
static void netset_ips4_func(void*);
static void netset_port_func(void*);
static void netset_save_func(void*);
static void netset_exit_func(void*);

static networkset_t networkset;// = { IP2UINT(10,1,2,130), IP2UINT(10,1,2,140), 9999 };
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int network_cfg_changed = 0;

inpfoc_funclist_t inpfoc_netset_funclist[NETSET_NUM_ITEMS+1] = 
{
  { NETSET_NONE,       netset_none_func         },
  { NETSET_IPM1,       netset_ipm1_func         },
  { NETSET_IPM2,       netset_ipm2_func         },
  { NETSET_IPM3,       netset_ipm3_func         },
  { NETSET_IPM4,       netset_ipm4_func         },
  { NETSET_IPS1,       netset_ips1_func         },
  { NETSET_IPS2,       netset_ips2_func         },
  { NETSET_IPS3,       netset_ips3_func         },
  { NETSET_IPS4,       netset_ips4_func         },
  { NETSET_PORT,       netset_port_func         },
  { NETSET_SAVE,       netset_save_func         },
  { NETSET_EXIT,       netset_exit_func         },
  { -1       ,         netset_none_func         }, // -1 must be last
};

static void netset_none_func(void * parg)
{

}

static void netset_ipm1_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  networkset_t nws;
  int v;
  char s[30];

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);
  v = (nws.ipaddr & 0xff000000) >> 24;

  v += pcmd->delta;
  pit = inpfoc_find(INPFOC_NETSET, NETSET_IPM1);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.ipaddr &= 0x00ffffff;
  nws.ipaddr |= (v << 24);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  network_cfg_changed = 1;
  inpfoc_enable(INPFOC_NETSET, NETSET_SAVE);
}

static void netset_ipm2_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  networkset_t nws;
  int v;
  char s[30];

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);
  v = (nws.ipaddr & 0x00ff0000) >> 16;

  v += pcmd->delta;
  pit = inpfoc_find(INPFOC_NETSET, NETSET_IPM2);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.ipaddr &= 0xff00ffff;
  nws.ipaddr |= (v << 16);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  network_cfg_changed = 1;
  inpfoc_enable(INPFOC_NETSET, NETSET_SAVE);
}

static void netset_ipm3_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  networkset_t nws;
  int v;
  char s[30];

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);
  v = (nws.ipaddr & 0x0000ff00) >> 8;

  v += pcmd->delta;
  pit = inpfoc_find(INPFOC_NETSET, NETSET_IPM3);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.ipaddr &= 0xffff00ff;
  nws.ipaddr |= (v << 8);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  network_cfg_changed = 1;
  inpfoc_enable(INPFOC_NETSET, NETSET_SAVE);
}

static void netset_ipm4_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  networkset_t nws;
  int v;
  char s[30];

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);
  v = (nws.ipaddr & 0x000000ff) >> 0;

  v += pcmd->delta;
  pit = inpfoc_find(INPFOC_NETSET, NETSET_IPM4);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.ipaddr &= 0xffffff00;
  nws.ipaddr |= (v << 0);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  network_cfg_changed = 1;
  inpfoc_enable(INPFOC_NETSET, NETSET_SAVE);
}

static void netset_ips1_func(void * parg)
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
  pit = inpfoc_find(INPFOC_NETSET, NETSET_IPS1);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.serverip &= 0x00ffffff;
  nws.serverip |= (v << 24);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  network_cfg_changed = 1;
  inpfoc_enable(INPFOC_NETSET, NETSET_SAVE);
}

static void netset_ips2_func(void * parg)
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
  pit = inpfoc_find(INPFOC_NETSET, NETSET_IPS2);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.serverip &= 0xff00ffff;
  nws.serverip |= (v << 16);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  network_cfg_changed = 1;
  inpfoc_enable(INPFOC_NETSET, NETSET_SAVE);
}

static void netset_ips3_func(void * parg)
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
  pit = inpfoc_find(INPFOC_NETSET, NETSET_IPS3);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.serverip &= 0xffff00ff;
  nws.serverip |= (v << 8);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  network_cfg_changed = 1;
  inpfoc_enable(INPFOC_NETSET, NETSET_SAVE);
}

static void netset_ips4_func(void * parg)
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
  pit = inpfoc_find(INPFOC_NETSET, NETSET_IPS4);
  assert(pit);

  while (v < 0) v += 256;
  v %= 256;

  nws.serverip &= 0xffffff00;
  nws.serverip |= (v << 0);
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  network_cfg_changed = 1;
  inpfoc_enable(INPFOC_NETSET, NETSET_SAVE);
}

static void netset_port_func(void * parg)
{
  inpfoc_item_t *pit;
  inpfoc_cmd_t *pcmd = (inpfoc_cmd_t *) parg;
  networkset_t nws;
  int v;
  char s[30];

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);
  v = nws.portno;

  v += pcmd->delta;
  pit = inpfoc_find(INPFOC_NETSET, NETSET_PORT);
  assert(pit);

  if (v < 0) v = 99999;
  v %= 100000;

  nws.portno = v;
  network_set(&nws);

  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(pit, s);

  network_cfg_changed = 1;
  inpfoc_enable(INPFOC_NETSET, NETSET_SAVE);
}

static void netset_save_func(void * parg)
{
  networkset_t nws;
  net_cfg_t net_cfg;

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);

  // network_cfg_save
  assert( sizeof(networkset_t) == sizeof(net_cfg_t) );
  memcpy(&net_cfg, &nws, sizeof(net_cfg_t));
  if ( stconf_write(STCONF_NETWORK, &net_cfg, sizeof(net_cfg_t)) > 0 );

  if (network_cfg_changed)
  {
#ifdef ARM
    char cmd[200];
    sprintf(cmd, "fw_setenv ipaddr %d.%d.%d.%d > /dev/null", (nws.ipaddr & 0xff000000) >> 24, (nws.ipaddr & 0x00ff0000) >> 16, (nws.ipaddr & 0x0000ff00) >> 8, (nws.ipaddr & 0x000000ff) >> 0);
    system(cmd);

   // system("ifconfig eth0 down");
   // sprintf(cmd, "ifconfig eth0 %d.%d.%d.%d up", (nws.ipaddr & 0xff000000) >> 24, (nws.ipaddr & 0x00ff0000) >> 16, (nws.ipaddr & 0x0000ff00) >> 8, (nws.ipaddr & 0x000000ff) >> 0);
#endif

    csio_deinit();
    csio_init();
  }

  network_cfg_changed = 0;
  inpfoc_disable(INPFOC_NETSET, NETSET_SAVE);
  inpfoc_set(INPFOC_NETSET, NETSET_EXIT);
}


static void netset_exit_func(void * parg)
{
  networkset_t nws;
  net_cfg_t net_cfg;

  memset(&nws, 0, sizeof(networkset_t));
  network_get(&nws);

  // network_cfg_save
  assert( sizeof(networkset_t) == sizeof(net_cfg_t) );
  memcpy(&net_cfg, &nws, sizeof(net_cfg_t));
  if ( stconf_write(STCONF_NETWORK, &net_cfg, sizeof(net_cfg_t)) > 0 );

  uframe_command(UFRAME_CHANGE, (void*)MENU_SETTINGS);
}

void menu_networkset_openproc(void)
{
  networkset_t nws;
  int v;
  char s[40];

  network_get(&nws);

#if defined (ARM)
  char cmd[200], buf[100];
  FILE * pipe;
  sprintf(cmd, "/usr/local/bin/fw_printenv 2>/dev/null | grep ipaddr | sed 's/ipaddr=//g'");
  if ((pipe = popen(cmd, "r")) != NULL)
  {
    if (fgets(buf, BUFSIZ, pipe) != NULL)
    {
      int i1, i2, i3, i4;
      sscanf(buf, "%d.%d.%d.%d", &i1, &i2, &i3, &i4);
      nws.ipaddr = IP2UINT(i1,i2,i3,i4);
    }
    else
    {
      error("\"ipaddr\" not found in env\n");
    }
    pclose(pipe);
  }
  network_set(&nws);
#endif

  v = (nws.ipaddr & 0xff000000) >> 24;
  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NETSET, NETSET_IPM1), s);
  v = (nws.ipaddr & 0x00ff0000) >> 16;
  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NETSET, NETSET_IPM2), s);
  v = (nws.ipaddr & 0x0000ff00) >> 8;
  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NETSET, NETSET_IPM3), s);
  v = (nws.ipaddr & 0x000000ff) >> 0;
  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NETSET, NETSET_IPM4), s);

  v = (nws.serverip & 0xff000000) >> 24;
  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NETSET, NETSET_IPS1), s);
  v = (nws.serverip & 0x00ff0000) >> 16;
  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NETSET, NETSET_IPS2), s);
  v = (nws.serverip & 0x0000ff00) >> 8;
  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NETSET, NETSET_IPS3), s);
  v = (nws.serverip & 0x000000ff) >> 0;
  snprintf(s, sizeof(s), "%d", v);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NETSET, NETSET_IPS4), s);

  snprintf(s, sizeof(s), "%d", nws.portno);
  inpfoc_wnd_setcaption(inpfoc_find(INPFOC_NETSET, NETSET_PORT), s);

  network_cfg_changed = 0;
  inpfoc_disable(INPFOC_NETSET, NETSET_SAVE);
  inpfoc_set(INPFOC_NETSET, NETSET_EXIT);
}

void network_get(networkset_t * pnws)
{
  pthread_mutex_lock(&mutex);
 // rwlock_lock_read(&nws_lock);

  if (pnws) *pnws = networkset;

  pthread_mutex_unlock(&mutex);
 // rwlock_unlock_read(&nws_lock);
}

void network_set(networkset_t * pnws)
{
 // rwlock_lock_write(&nws_lock);
  pthread_mutex_lock(&mutex);

  if (pnws) memcpy(&networkset, pnws, sizeof(networkset_t));

  pthread_mutex_unlock(&mutex);
 // rwlock_unlock_write(&nws_lock);
}

void network_ini(networkset_t * pnws)
{
  networkset_t nws;
  net_cfg_t net_cfg;

  network_ini_def(pnws);

  memset(&nws, 0, sizeof(networkset_t));
  if ( stconf_read(STCONF_NETWORK, &net_cfg, sizeof(net_cfg_t)) > 0 )
  {
    assert( sizeof(net_cfg_t) == sizeof(networkset_t) );
    memcpy(&nws, &net_cfg, sizeof(networkset_t));
    network_set(&nws);
  }
  else
    return;

  if (pnws)
    memcpy(pnws, &nws, sizeof(networkset_t));
}

void network_ini_def(networkset_t * pnws)
{
  networkset_t nws;

  nws.ipaddr   = IP2UINT(192,168,10,102);
  nws.serverip = IP2UINT(192,168,10,101);
  nws.portno   = 9999;

  network_set(&nws);

  if (pnws)
    memcpy(pnws, &nws, sizeof(networkset_t));
}

void network_deini(void)
{
  networkset_t nws;
  net_cfg_t net_cfg;

  network_get(&nws);
  assert( sizeof(net_cfg_t) == sizeof(networkset_t) );
  memcpy(&net_cfg, &nws, sizeof(networkset_t));

  if ( stconf_write(STCONF_NETWORK, &net_cfg, sizeof(net_cfg_t)) > 0 );
}
