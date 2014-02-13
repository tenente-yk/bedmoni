#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "stconf.h"
#include "sched.h"
#include "uframe.h"
#include "lowbatshutdown.h"

static void hide_lowbatshutdown_popup(void)
{
  uframe_command(UFRAME_DESTROY, NULL);
}

void menu_lowbatshutdown_openproc(void)
{
  sched_start(SCHED_ANY, 5*1000, hide_lowbatshutdown_popup, SCHED_DO_ONCE);
}
