#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "uframe.h"
#include "cframe.h"
#include "tframe.h"
#include "lframe.h"
#include "mframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "shutdown.h"
#include "crbpio.h"

static void shutdown_none_func(void*);
static void shutdown_yes_func(void*);
static void shutdown_no_func(void*);

inpfoc_funclist_t inpfoc_shutdown_funclist[SHUTDOWN_NUM_ITEMS+1] = 
{
  { SHUTDOWN_NONE,    shutdown_none_func         },
  { SHUTDOWN_YES,     shutdown_yes_func          },
  { SHUTDOWN_NO,      shutdown_no_func           },
  { -1       ,        shutdown_none_func         }, // -1 must be last
};

static void shutdown_none_func(void * parg)
{

}

static void shutdown_yes_func(void * parg)
{
  safe_exit(EXIT_OK);
}

static void shutdown_no_func(void * parg)
{
  uframe_command(UFRAME_DESTROY, NULL);
}

void menu_shutdown_openproc(void)
{
  inpfoc_set(INPFOC_SHUTDOWN, SHUTDOWN_NO);
}

