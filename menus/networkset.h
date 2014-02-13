#ifndef __NETWORKSET_H
#define __NETWORKSET_H

#include "inpfoc.h"

#define IP2UINT(ip1,ip2,ip3,ip4) \
 ( ((ip1&0xff) << 24) | ((ip2&0xff) << 16) | ((ip3&0xff) << 8) | ((ip4&0xff) << 0) )

enum
{
  NETSET_NONE = 0,   // 0 - have to be not used
  NETSET_IPM1,       // 1
  NETSET_IPM2,       // 2
  NETSET_IPM3,       // 3
  NETSET_IPM4,       // 4
  NETSET_IPS1,       // 5
  NETSET_IPS2,       // 6
  NETSET_IPS3,       // 7
  NETSET_IPS4,       // 8
  NETSET_PORT,       // 9
  NETSET_SAVE,       // 10
  NETSET_EXIT,       // 11
  NETSET_NUM_ITEMS,  // 12
};

#pragma pack(1)
typedef struct
{
  unsigned int ipaddr;
  unsigned int serverip;
  int          portno;
} networkset_t;
#pragma pack()

typedef networkset_t net_cfg_t;

extern inpfoc_funclist_t inpfoc_netset_funclist[NETSET_NUM_ITEMS+1];
extern char menu_networkset[];

void menu_networkset_openproc(void);

void network_get(networkset_t * pnws);
void network_set(networkset_t * pnws);
void network_ini(networkset_t * pnws);
void network_ini_def(networkset_t * pnws);
void network_deini(void);

#endif // __NETWORKSET_H
