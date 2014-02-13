#include "bedmoni.h"
#include "uframe.h"
#include "dialog.h"

static void dialog_none_func(void*);
static void dialog_ok_func(void * parg);

inpfoc_funclist_t inpfoc_dialog_funclist[DIALOG_NUM_ITEMS+1] = 
{
  { DIALOG_NONE,        dialog_none_func        },
  { DIALOG_YES,         dialog_none_func        },
  { DIALOG_NO,          dialog_none_func        },
  { DIALOG_OK,          dialog_ok_func          },
  { -1       ,          dialog_none_func        }, // -1 must be last
};

static void dialog_none_func(void * parg)
{

}

static void dialog_ok_func(void * parg)
{
  uframe_command(UFRAME_DESTROY, NULL);
}

