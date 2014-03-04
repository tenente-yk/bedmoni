#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif
#include "udp.h"
#include "dio.h"
#include "dproc.h"
#include "debug.h"
#include "cs_demo.h"

volatile int exit_flag = 0;

int main(int argc, char **argv)
{
#if defined (DATA_FROM_FILE)
  FILE * f;
#endif
  unsigned char buf[500];

  debug_init();

  debug("\r\ncs_demo\r\n");
  debug("server started...\r\n");

  dproc_init();

#if defined (DATA_FROM_FILE)
  f = fopen("demodata55.bin", "rb");
#else
  udp_server_init();
#endif

  while (!exit_flag)
  {
    int nr;
#if defined (DATA_FROM_FILE)
    nr = fread(buf,1,sizeof(buf),f);
    if (feof(f)) break;
#else
    nr = udp_read(buf, sizeof(buf));
#endif

    if (nr > 0) dio_process_data(buf, nr);
//    printf("nr=%d\n", nr);
#ifdef UNIX
    usleep(10*1000);
#endif
#ifdef WIN32
    Sleep(10);
#endif
  }

#if defined (DATA_FROM_FILE)
  fclose(f);
#else
  udp_server_deinit();
#endif

  return 0;
}
