#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "bedmoni.h"
#include "stconf.h"
#include "uframe.h"
#include "inpfoc.h"
#include "inpfoc_wnd.h"
#include "lowbatcharge.h"

static void lowbat_none_func(void*);
static void lowbat_ok_func(void*);

//static int lowbat_warning_confirmed = 0;

inpfoc_funclist_t inpfoc_lowbat_funclist[LOWBAT_NUM_ITEMS+1] = 
{
  { LOWBAT_NONE,      lowbat_none_func        },
  { LOWBAT_OK,        lowbat_ok_func          },
  { -1       ,        lowbat_none_func        }, // -1 must be last
};

static void lowbat_none_func(void * parg)
{

}

static void lowbat_ok_func(void * parg)
{
//  lowbat_warning_confirmed = 1;
  uframe_command(UFRAME_DESTROY, NULL);
}

//int lowbat_confirmed(void)
//{
//  return lowbat_warning_confirmed;
//}

void menu_lowbat_openproc(void)
{
  //
  inpfoc_set(INPFOC_LOWBAT, LOWBAT_OK);
}
