#ifdef UNIX
#include <unistd.h>
#endif
#include <stdio.h>
#include "dview.h"
#include "gview.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "tframe.h"
#include "lframe.h"
#include "bedmoni.h"

int  gview_init(void)
{
  inpfoc_wnd_init();

  return 0;
}

void gview_deinit(void)
{
  inpfoc_wnd_deinit();
}

void gview_update(void)
{
  static int cnt = 0;

  if (cnt & 0x7)
    inpfoc_update();

  dview_draw();

  cnt ++;
}
