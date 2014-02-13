#ifdef WIN32
//#include <windows.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

#include "inpfoc.h"
#include "inpfoc_wnd.h"

#include "bedmoni.h"
#include "mainmenu.h"
#include "datetime.h"
#include "general.h"
#include "patient.h"
#include "settings.h"
#include "alarms.h"
#include "trnmenu.h"
#include "modules.h"
#include "defaultset.h"
#include "networkset.h"
#include "ecgset.h"
#include "spo2set.h"
#include "respset.h"
#include "nibpset.h"
#include "t1t2set.h"
#include "tblmenu.h"
#include "tableset.h"
#include "printset.h"
#include "trendset.h"
#include "shutdown.h"
#include "password.h"
#include "patientnew.h"
#include "stset.h"
#include "arrhset.h"
#include "nibpcal.h"
#include "swupdate.h"
#include "lowbatcharge.h"
#include "langsel.h"
#include "dispset.h"
#include "demoset.h"
#include "dialog.h"
#include "leaktst.h"
#include "veriftst.h"
#include "co2set.h"
#include "co2iniset.h"
#include "evlogmenu.h"
#include "evlogset.h"
#include "appinfo.h"
#include "adminset.h"

static int id_curr = INPFOC_MAIN;
static volatile int inpfoc_newstate = 0;
inpfoc_item_t * if_item[INPFOC_MAX_NUM]; // = {NULL, NULL, NULL};
inpfoc_funclist_t * funclist[INPFOC_MAX_NUM] = 
{
  NULL,
  (inpfoc_funclist_t*) &inpfoc_main_funclist,
  (inpfoc_funclist_t*) &inpfoc_general_funclist,
  (inpfoc_funclist_t*) &inpfoc_dt_funclist,
  (inpfoc_funclist_t*) &inpfoc_pat_funclist,
  (inpfoc_funclist_t*) &inpfoc_set_funclist,
  (inpfoc_funclist_t*) &inpfoc_alr_funclist,
  (inpfoc_funclist_t*) &inpfoc_trn_funclist,
  (inpfoc_funclist_t*) &inpfoc_mod_funclist,
  (inpfoc_funclist_t*) &inpfoc_defset_funclist,
  (inpfoc_funclist_t*) &inpfoc_netset_funclist,
  (inpfoc_funclist_t*) &inpfoc_ecgset_funclist,
  (inpfoc_funclist_t*) &inpfoc_spo2set_funclist,
  (inpfoc_funclist_t*) &inpfoc_respset_funclist,
  (inpfoc_funclist_t*) &inpfoc_nibpset_funclist,
  (inpfoc_funclist_t*) &inpfoc_t1t2set_funclist,
  (inpfoc_funclist_t*) &inpfoc_tbl_funclist,
  (inpfoc_funclist_t*) &inpfoc_tblset_funclist,
  (inpfoc_funclist_t*) &inpfoc_prnset_funclist,
  (inpfoc_funclist_t*) &inpfoc_trnset_funclist,
  (inpfoc_funclist_t*) &inpfoc_shutdown_funclist,
  (inpfoc_funclist_t*) &inpfoc_password_funclist,
  (inpfoc_funclist_t*) &inpfoc_patnew_funclist,
  (inpfoc_funclist_t*) &inpfoc_stset_funclist,
  (inpfoc_funclist_t*) &inpfoc_arrhset_funclist,
  (inpfoc_funclist_t*) &inpfoc_nibpcal_funclist,
  (inpfoc_funclist_t*) &inpfoc_swupdate_funclist,
  (inpfoc_funclist_t*) &inpfoc_lowbat_funclist,
  (inpfoc_funclist_t*) &inpfoc_langsel_funclist,
  (inpfoc_funclist_t*) &inpfoc_dispset_funclist,
  (inpfoc_funclist_t*) &inpfoc_demoset_funclist,
  (inpfoc_funclist_t*) &inpfoc_dialog_funclist,
  (inpfoc_funclist_t*) &inpfoc_leaktest_funclist,
  (inpfoc_funclist_t*) &inpfoc_veriftest_funclist,
  (inpfoc_funclist_t*) &inpfoc_co2set_funclist,
  (inpfoc_funclist_t*) &inpfoc_co2iniset_funclist,
  (inpfoc_funclist_t*) &inpfoc_evlog_funclist,
  (inpfoc_funclist_t*) &inpfoc_evlogset_funclist,
  (inpfoc_funclist_t*) &inpfoc_appinfo_funclist,
  (inpfoc_funclist_t*) &inpfoc_adminset_funclist,
};

static void inpfoc_dbg(char * fmt, ...)
{
  va_list  args;
  char s[200];

  va_start(args, fmt);
  vsprintf(s, fmt, args);

#ifdef WIN32
  //MessageBox(0,s,"Info",0);
  //yfprintf(stdout, s);
  yprintf(s);
#endif
#if defined (CONSOLE) || defined UNIX
  printf("%s\n", s);
#endif

  va_end(args);
}

inpfoc_item_t * inpfoc_find(int id, int item)
{
  inpfoc_item_t *ptr;
  if (if_item[id] == NULL) return NULL;
  ptr = if_item[id];
  do
  {
    if (ptr->item == item) return ptr;
    ptr = ptr->next;
  } while (!ptr->last);
  if (ptr->item == item) return ptr;
//printf("id=%d, item=%d NULL\n", id, item);
  return NULL;
}

inpfoc_item_t * inpfoc_find_first(int id)
{
  inpfoc_item_t *ptr;
  if (if_item[id] == NULL) return NULL;
  ptr = if_item[id];
  do
  {
    if (ptr->first) return ptr;
    ptr = ptr->next;
  } while (!ptr->last);
  if (ptr->first) return ptr;
  return NULL;
}

static inpfoc_item_t * inpfoc_find_last(int id)
{
  inpfoc_item_t *ptr;
  if (if_item[id] == NULL) return NULL;
  ptr = if_item[id];
  do
  {
    if (ptr->last) return ptr;
    ptr = ptr->next;
  } while (!ptr->last);
  if (ptr->last) return ptr;
  return NULL;
}

inpfoc_item_t * inpfoc_find_active(int id)
{
  inpfoc_item_t *ptr;
  if (if_item[id] == NULL) return NULL;
  ptr = if_item[id];
  do
  {
    if (ptr->active) return ptr;
    ptr = ptr->next;
  } while (!ptr->last);
  if (ptr->active) return ptr;
  return NULL;
}

inpfoc_item_t * inpfoc_find_selected(int id)
{
  inpfoc_item_t *ptr;
  if (if_item[id] == NULL) return NULL;
  ptr = if_item[id];
  do
  {
    if (ptr->selected) return ptr;
    ptr = ptr->next;
  } while (!ptr->last);
  if (ptr->selected) return ptr;
  return NULL;
}

void inpfoc_add(int id, int item, void * parg)
{
  inpfoc_item_t *pifi, *ptr, *ptr2;
  int inpfoc_type = INPFOC_TYPE_PUSH;

  assert (parg);
  inpfoc_type = 
  (
    ((inpfoc_wnd_t*)parg)->type == INPFOC_WND_LIST || 
    ((inpfoc_wnd_t*)parg)->type == INPFOC_WND_SPINBUTTON || 
    ((inpfoc_wnd_t*)parg)->type == INPFOC_WND_SCROLLBAR || 
    ((inpfoc_wnd_t*)parg)->type == INPFOC_WND_SLIDER
  ) ? INPFOC_TYPE_LIST : INPFOC_TYPE_PUSH;

  pifi = (inpfoc_item_t*) malloc(sizeof(inpfoc_item_t));
  assert(pifi);
  if (if_item[id] == NULL)
  {
    if (pifi)
    {
      if_item[id] = pifi;
      memset(if_item[id], 0, sizeof(*if_item[id]));
      if_item[id]->parg = parg;
      if_item[id]->last = 1;
      if_item[id]->first = 1;
      if_item[id]->item = item;
     // if_item[id]->active = 0;
      if_item[id]->next = if_item[id];
      if_item[id]->prev = if_item[id];
      if_item[id]->type = inpfoc_type;
    }
    return;
  }
  // if_item[id] != NULL
  if (pifi)
  {
    memset(pifi, 0, sizeof(*pifi));
    ptr = inpfoc_find_first(id);
    ptr2 = inpfoc_find_last(id);
    if (!ptr || !ptr2) return;
    ptr2->next = pifi;
    ptr2->last = 0;
    pifi->parg = parg;
    pifi->item = item;
    pifi->next = ptr;
    pifi->prev = ptr2;
    pifi->first = 0;
    pifi->last = 1;
    pifi->type = inpfoc_type;

    ptr2 = inpfoc_find_last(id);
    if (ptr2)
      ptr->prev = ptr2;
   // pifi->active = 0;
  }
}

void inpfoc_ins(int id, int item, void * parg, int item0)
{
  inpfoc_item_t *pifi, *ptr; //, *ptr2;
  int inpfoc_type = INPFOC_TYPE_PUSH;

  assert (parg);
  inpfoc_type = 
  (
    ((inpfoc_wnd_t*)parg)->type == INPFOC_WND_LIST || 
    ((inpfoc_wnd_t*)parg)->type == INPFOC_WND_SPINBUTTON || 
    ((inpfoc_wnd_t*)parg)->type == INPFOC_WND_SCROLLBAR || 
    ((inpfoc_wnd_t*)parg)->type == INPFOC_WND_SLIDER
  ) ? INPFOC_TYPE_LIST : INPFOC_TYPE_PUSH;

//printf("inserted list %d\n", (((inpfoc_wnd_t*)parg)->type == INPFOC_WND_LIST) ? INPFOC_TYPE_LIST : INPFOC_TYPE_PUSH);

  pifi = (inpfoc_item_t*) malloc(sizeof(inpfoc_item_t));
  assert(pifi);
  if (if_item[id] == NULL)
  {
    if (pifi)
    {
      if_item[id] = pifi;
      memset(if_item[id], 0, sizeof(*if_item[id]));
      if_item[id]->parg = parg;
      if_item[id]->last = 1;
      if_item[id]->first = 1;
      if_item[id]->item = item;
     // if_item[id]->active = 0;
      if_item[id]->next = if_item[id];
      if_item[id]->prev = if_item[id];
      if_item[id]->type = inpfoc_type;
//printf("if_item[id]->type = %d, sel %d\n", if_item[id]->type, if_item[id]->selected);
    }
    return;
  }
  // if_item[id] != NULL
  if (pifi)
  {
    memset(pifi, 0, sizeof(*pifi));
    ptr = inpfoc_find(id, item0);
    if (!ptr)
    {
      ptr = inpfoc_find_first(id);
      if (!ptr)
        return;
      // insert as first item
      ptr->prev->next = pifi;
      pifi->next = ptr;
      pifi->prev = ptr->prev;;
      ptr->prev = pifi;
      if (ptr->first)
      {
        ptr->first = 0;
        pifi->first = 1;
        if_item[id] = pifi;
      }
      pifi->parg = parg;
      pifi->item = item;
      pifi->last = 0;
      pifi->type = inpfoc_type;

     // inpfoc_list(id);
      return;
    }
    // insert item after item0
    pifi->next = ptr->next;
    ptr->next->prev = pifi;
    ptr->next = pifi;
    if (ptr->last)
    {
      ptr->last = 0;
      pifi->last = 1;
    }
    pifi->parg = parg;
    pifi->item = item;
    pifi->prev = ptr;
    pifi->first = 0;
    pifi->type = inpfoc_type;
  }
 // inpfoc_list(id);
}

void inpfoc_del(int id, int item)
{
  inpfoc_item_t *ptr, *ptr2, *ptr3;
  ptr = inpfoc_find(id, item);
  if (!ptr) return;

//debug("%s: %d %d\n", __FUNCTION__, id, item);

  ptr->deleted = 1;

  inpfoc_wnd_destroy(ptr);

  ptr2 = ptr->prev;
  ptr3 = ptr->next;
  ptr2->next = ptr3;
  ptr3->prev = ptr2;
  if (ptr == ptr2)
  {
    if_item[id] = NULL;
   // free(ptr->parg);
    free(ptr);
    return;
  }
  if (ptr->first)
  {
    if_item[id] = ptr3;
    ptr3->first = 1;
  }
  if (ptr->last)  ptr2->last = 1;
 // free(ptr->parg);
  free(ptr);

 // if (ptr == ptr2) if_item[id] = NULL;

 // inpfoc_list(id);
}

void inpfoc_list(int id)
{
  inpfoc_item_t *ptr;

 // inpfoc_dbg("inpfoc_list %X\n", if_item[id]);
  if (if_item[id] == NULL) return;
  ptr = if_item[id];
  do
  {
    inpfoc_dbg("%02d: %X ->%X %X<-  %X  %d %d    %d %d %d %d\n", ptr->item, (unsigned int)ptr, (unsigned int)ptr->next, (unsigned int)ptr->prev, (unsigned int)ptr->parg, ptr->first, ptr->last, ptr->active, ptr->selected, ptr->pressed, ptr->rolled);
    if (ptr->last) break;
    ptr = ptr->next;
  } while (1);
  inpfoc_dbg("\n");
}

void inpfoc_set(int id, int item)
{
  inpfoc_item_t *ptr;

  ptr = inpfoc_find_active(id);
  if (ptr)
  {
    ptr->active = 0;
    ptr->changed = 1;
  }

#if 0 // may be not needed, if disable it - need to test with quick roll and press
  if (if_item[id] == NULL) return;
  for (ptr=if_item[id]; !ptr->last; ptr=ptr->next)
  {
    ptr->active = 0;
  }
#endif

  ptr = inpfoc_find(id, item);
  if (!ptr) ptr = inpfoc_find_first(id);
  if (!ptr) return;
  ptr->active = 1;
  ptr->changed = 1;
  inpfoc_newstate = 1;
}

void inpfoc_press(void)
{
  inpfoc_item_t *ptr;
  int id;
  id = id_curr;

  if (if_item[id] == NULL) return;
  for (ptr=if_item[id]; !ptr->last; ptr=ptr->next)
  {
    ptr->pressed = 0;
  }

  ptr = inpfoc_find_active(id);
  if (!ptr) return;
 // inpfoc_dbg("PRESS\n");
  ptr->pressed = 1;
  ptr->changed = 1;
  ptr->delta = 0;
  inpfoc_newstate = 1;

//inpfoc_list(id);
}

void inpfoc_shl(int delta)
{
  inpfoc_item_t *ptr;
  int id;

  if (delta == 0) return;

  inpfoc_newstate = 1;

  id = id_curr;
  ptr = inpfoc_find_selected(id);
  if (ptr)
  {
   // inpfoc_proc(id, ptr->item, -1);
    ptr->delta += -1*delta;
    ptr->rolled = 1;
    return;
  }
  ptr = inpfoc_find_active(id);
  if (ptr)
  {
  }
  else
  {
    inpfoc_set(id, -1);
    ptr = inpfoc_find_first(id);
    if (!ptr) return;
  }
  while (delta--)
  {
    if (ptr->prev->disabled)
    {
      delta++;
      ptr->active = 0;
      ptr->changed = 1;
      ptr = ptr->prev;
      continue;
    }
    ptr->active = 0;
    ptr->changed = 1;
    ptr->prev->active = 1;
    ptr->prev->changed = 1;
    ptr = ptr->prev;
  }
}

void inpfoc_shr(int delta)
{
  inpfoc_item_t *ptr;
  int id;

  if (delta == 0) return;

  inpfoc_newstate = 1;

  id = id_curr;
  ptr = inpfoc_find_selected(id);
  if (ptr)
  {
//printf("selected %d\n", ptr->item);
   // inpfoc_proc(id, ptr->item, +1);
    ptr->delta += +1*delta;
    ptr->rolled = 1;
    return;
  }
  ptr = inpfoc_find_active(id);
  if (ptr)
  {
  }
  else
  {
    inpfoc_set(id, -1);
    ptr = inpfoc_find_last(id);
    if (!ptr) return;
  }
  while (delta--)
  {
    if (ptr->next->disabled)
    {
      delta++;
      ptr->active = 0;
      ptr->changed = 1;
      ptr = ptr->next;
      continue;
    }
    ptr->active = 0;
    ptr->changed = 1;
    ptr->next->active = 1;
    ptr->next->changed = 1;
    ptr = ptr->next;
  }
}

void inpfoc_invalidate(int id)
{
#if 0
  inpfoc_item_t *ptr;
  ptr = inpfoc_find_first(id);
  if (!ptr) return;
  for(; !ptr->last; ptr=ptr->next)
  {
    ptr->changed = 1;
  }
  ptr->changed = 1; // do it for the last item too
#endif
  debug("%s: out of realization\n", __FUNCTION__);
  inpfoc_newstate = 1;
}

void inpfoc_disable(int id, int item)
{
  inpfoc_item_t *ptr;

  ptr = inpfoc_find(id, item);
  if (!ptr) return;

  ptr->disabled = 1;
  ptr->changed = 1;
  inpfoc_newstate = 1;
}

void inpfoc_enable(int id, int item)
{
  inpfoc_item_t *ptr;

  ptr = inpfoc_find(id, item);
  if (!ptr) return;

  ptr->disabled = 0;
  ptr->changed = 1;

  inpfoc_newstate = 1;

 // inpfoc_wnd_enable(ptr);
}

void inpfoc_focus(int id)
{
#if 1
  // TODO: disabled by YK !!! - reason: belye kvadraty !!!
  return;
#endif
  inpfoc_item_t *ptr;
  ptr = inpfoc_find_active(id);
  if (!ptr)
  {
    ptr = inpfoc_find_first(id);
    if (ptr) ptr->active = 1;
  }
  if (!ptr) return;
  ptr->changed = 1;
  inpfoc_newstate = 1;
}

void inpfoc_reload(int id)
{
  inpfoc_item_t *ptr;
  ptr = inpfoc_find_first(id);
  if (!ptr) return;
  for(; !ptr->last; ptr=ptr->next)
  {
    inpfoc_wnd_setcaption(ptr, NULL);
  }
  inpfoc_wnd_setcaption(ptr, NULL);
}

void inpfoc_update(void)
{
  inpfoc_item_t *ptr;
  int id;

  if (!inpfoc_newstate) return;
  inpfoc_newstate = 0;

 // inpfoc_list(id);

  // may be need to check all id's
  id = id_curr;

// debug("%s %d\n", __FUNCTION__, id);

  if (if_item[id] == NULL) return;
  ptr = if_item[id];
  do
  {
    if (ptr->changed)
    {
      if (ptr->selected)
      {

      }
      else if (ptr->active)
      {
       // inpfoc_dbg("active %X %d %d\n", (UINT)ptr->parg, ptr->first, ptr->last);
        inpfoc_wnd_select(ptr);
      }
      else
      {
        if (ptr->disabled)
        {
         // inpfoc_wnd_deselect(ptr);
          inpfoc_wnd_disable(ptr);
        }
        else
        {
          inpfoc_wnd_deselect(ptr);
        }
      }
      ptr->changed = 0;
    }
    if (ptr->pressed)
    {
     // printf("pressed %X %d %d %d\n", (UINT)ptr->parg, ptr->first, ptr->last, ptr->item);
     // printf("%d %d\n", id, ptr->item);
//debug("%d %d %d\n", id, ptr->item, ptr);
      ptr->pressed = 0;
      if (ptr->type == INPFOC_TYPE_PUSH)
      {
        inpfoc_wnd_check(ptr, IWC_TOGGLED);
        inpfoc_proc(id, ptr->item, ptr->delta);
        break;
      }
      if (ptr->type == INPFOC_TYPE_LIST && ptr->active)
      {
        ptr->selected = !ptr->selected;
//printf("ptr->selected = %d\n", ptr->selected);
        if (ptr->selected)
          inpfoc_wnd_press(ptr);
        else
          inpfoc_wnd_select(ptr);
      }
    }
    if (ptr->rolled)
    {
      ptr->rolled = 0;
      inpfoc_wnd_scroll(ptr, ptr->delta);
      inpfoc_proc(id, ptr->item, ptr->delta);
      ptr->delta = 0;
    }
#if 0 // maybe not needed
    ptr->pressed = 0;
#endif
    if (ptr->last) break;
    ptr=ptr->next;
  } while(1);
}

void inpfoc_proc(const int id, const int item, int delta)
{
  inpfoc_funclist_t * p_funclist;
  inpfoc_cmd_t inpfoc_cmd;

//debug("%s: %d %d\n", __FUNCTION__, id, item);

  p_funclist = funclist[id];//inpfoc_main_funclist;

  while(p_funclist->fno != -1)
  {
    if (p_funclist->fno == item)
    {
      inpfoc_cmd.delta = delta;
      p_funclist->func(&inpfoc_cmd);
      break;
    }
    p_funclist ++;
  }
}

void inpfoc_change(int id)
{
  if (id >= INPFOC_MAX_NUM)
  {
    fprintf(stderr, "%s invalid id %d\n", __FUNCTION__, id);
    return;
  }
 // if_item[id_curr] = NULL;
  id_curr = id;
}

int  inpfoc_getcurr(void)
{
  return id_curr;
}

void inpfoc_rm(int id)
{
 // inpfoc_item_t * ptr;
  while(inpfoc_find_first(id))
  {
    inpfoc_del(id, if_item[id]->item);
  }
}
