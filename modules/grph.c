/*! \file grph.c
 *  \brief Graphic primitives
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "lang.h"
#include "ykf.h"
#include "grph.h"

#include "bedmoni.h"

// static void * memsetw(void *s, int c, size_t n)
// {
//   size_t i;
//   unsigned short *s1 = (unsigned short *) s;
//   for (i=0; i<n; i++)
//     s1[i] = (unsigned short) c;
//   return s;
// }

static unsigned long bitmask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

static gui_font_t gui_fonts[NUM_FONTS] =
{
  { "ARIAL",  ARIAL_8,   "arial_8",   0 },
  { "ARIAL",  ARIAL_8B,  "arial_8B",  0 },
  { "ARIAL",  ARIAL_9,   "arial_9",   0 },
  { "ARIAL",  ARIAL_10,  "arial_10",  0 },
  { "ARIAL",  ARIAL_12,  "arial_12",  0 },
  { "ARIAL",  ARIAL_14,  "arial_14",  0 },
  { "ARIAL",  ARIAL_18,  "arial_18",  0 },
  { "ARIAL",  ARIAL_28,  "arial_28",  0 },
  { "ARIAL",  ARIAL_36,  "arial_36",  0 },
  { "ARIAL",  ARIAL_48,  "arial_48",  0 },
  { "ARIAL",  ARIAL_60,  "arial_60",  0 },
  { "MONOS",  MONOS_11,  "monos_11",  0 },
  { "MONOS",  MONOS_10,  "monos_10",  0 },
  { "ARIAL_N", ARIAL_N_12, "arial_n_12B", 0 },
  { "ARIAL_N", ARIAL_N_24, "arial_n_24B", 0 },
  { "ARIAL_N", ARIAL_N_30, "arial_n_30B", 0 },
  { "ARIAL_N", ARIAL_N_49, "arial_n_49B", 0 },
  { "ARIAL_N", ARIAL_N_70, "arial_n_70B", 0 },
  { "ARIAL_N", ARIAL_N_84, "arial_n_84B", 0 },
  { "ARIAL_N", ARIAL_N_114,"arial_n_114B",0 },
};

#if defined (FEATURE_BPP32) || defined (FEATURE_BPP24)
static unsigned char line[4*GRPH_MAX_LINE_LEN];
static void grph_filldata32(int fd, int x, int y, int cx, int cy, void * data);
#else
static unsigned char line[2*GRPH_MAX_LINE_LEN];
#endif

static int fnt[NUM_FONTS];

int grph_init(void)
{
  char s[200];
  int i, lang;

  memset(fnt, 0, sizeof(fnt[0]*NUM_FONTS));
  lang = lang_get();

  for(i=0; i<NUM_FONTS;i++)
  {
    sprintf(s, "%s/%s/%s/%s.%s", USBDISK_PATH, "fonts", lang_fontdir[lang], gui_fonts[i].fname, "ykf");
    fnt[i] = ykf_open(s);
    if (fnt[i] == 0)
    {
      sprintf(s, "%s/%s/%s/%s.%s", "/usr/local", "fonts", lang_fontdir[lang], gui_fonts[i].fname, "ykf");
      fnt[i] = ykf_open(s);
      if (fnt[i] == 0)
      {
        error("font %s/%s.%s not found\n", lang_fontdir[lang], gui_fonts[i].fname, "ykf");
      }
    }
  }

  return 0;
}

void grph_deinit(void)
{
  int i;

  for (i=0; i<NUM_FONTS; i++)
  {
    if (fnt[i]) ykf_close(fnt[i]);
  }
  memset(fnt, 0, sizeof(fnt[0]*NUM_FONTS));
}

int grph_create(PGSI pgsi, int flags)
{
  PGDC pgdc;
  int lfd;

  pgdc = (PGDC) calloc(1, sizeof(gdc_t));
  assert(pgdc);
  pgdc->xres = pgsi->xres;
  pgdc->yres = pgsi->yres;
  pgdc->bpp  = pgsi->bpp;
  if (pgdc->bpp == 1)
    pgdc->addr = calloc(pgdc->yres*(pgdc->xres+7)/8, 1);
  else
  if (pgdc->bpp == 8 || (pgdc->bpp == 16))
    pgdc->addr = calloc(pgdc->xres*pgdc->yres, (int)((pgdc->bpp+7)/8));
  else
#if defined (FEATURE_BPP32)
  if (pgdc->bpp == 32)
    pgdc->addr = calloc(pgdc->xres*pgdc->yres, (int)((pgdc->bpp+7)/8));
  else
#endif // FEATURE_BPP32
  {
    assert(0);
  }
  assert(pgdc->addr);
  pgdc->x    = 0;
  pgdc->y    = 0;
  pgdc->cx   = pgdc->xres;
  pgdc->cy   = pgdc->yres;
  pgdc->fgcolor = 0x0000;
  pgdc->bkcolor = 0xffff;
  pgdc->font_id = ARIAL_8B;
  pgdc->ps = PS_SOLID;
  lfd = (int) pgdc;
  return lfd;
}

void grph_destroy(int fd)
{
  PGDC pgdc = (PGDC) fd;
  if (pgdc)
  {
    if (pgdc->addr)
      free(pgdc->addr);
    free(pgdc);
  }
}

void grph_setfgcolor(int fd, unsigned long rgb)
{
  PGDC pgdc = (PGDC) fd;
  switch (pgdc->bpp)
  {
    case 16:
      pgdc->fgcolor = COLOR2PIXEL565(rgb);
      break;
#if defined (FEATURE_BPP32)
    case 32:
      pgdc->fgcolor = COLOR2PIXEL8888(rgb);
      break;
#endif // FEATURE_BPP32
    default:
      pgdc->fgcolor = COLOR2PIXEL565(rgb);
  }
}

void grph_setbkcolor(int fd, unsigned long rgb)
{
  PGDC pgdc = (PGDC) fd;
  switch (pgdc->bpp)
  {
    case 16:
      pgdc->bkcolor = COLOR2PIXEL565(rgb);
      break;
#if defined (FEATURE_BPP32)
    case 32:
      pgdc->bkcolor = COLOR2PIXEL8888(rgb);
      break;
#endif
    default:
      pgdc->bkcolor = COLOR2PIXEL565(rgb);
  }
}

void grph_setfont(int fd, unsigned int font_id)
{
  PGDC pgdc = (PGDC) fd;

  if (font_id >= NUM_FONTS) return;

  pgdc->font_id = font_id;
}

int  grph_setfont_sh(int fd, char* name, int h)
{
  int i;
  int found_id;
  int eps = 2000;

  assert(name);

  found_id = 0;

  for (i=0; i<NUM_FONTS; i++)
  {
    if (strcmp(gui_fonts[i].name, name) == 0)
    {
      int font_h;
      if (!fnt[i]) continue;
      font_h = ykf_get_font_h(fnt[i]);

      if (abs(font_h - h) < eps)
      {
        eps = abs(font_h - h);
        found_id = i;
      }
    }
  }

  grph_setfont(fd, found_id);
  return found_id;
}

void grph_select_pen(int fd, int ps)
{
  PGDC pgdc = (PGDC) fd;
  if (ps >= PS_NUM_STYLES) return;
  pgdc->ps = ps;
}

inline void lcd_point(int fd, int x, int y)
{
  PGDC pgdc = (PGDC) fd;

//printf("%s: %d %d %d %d\n", __FUNCTION__, x, y, pgdc->cx, pgdc->cy);

 /* assert( x >= 0 && x < pgdc->xres);
  assert( y >= 0 && y < pgdc->yres);
*/
  // x and y have to be already fit to dc
  if (x<0 || x>=pgdc->cx) return;
  if (y<0 || y>=pgdc->cy) return;

//printf("%d %d %d %d %d %d\n", pgdc->x, pgdc->cx, pgdc->xres, pgdc->y, pgdc->cy, pgdc->yres);
//printf("lcd_point %d %d %X %X\n", x, y, pgdc->addr, pgdc->addr + x * pgdc->bypp + y * pgdc->xres * pgdc->bypp);

  switch (pgdc->bpp)
  {
    case 1:
#if 1
      if (pgdc->fgcolor)
        *((unsigned char*) ((unsigned char*)pgdc->addr + y * ((pgdc->cx+7)/8) + x/8)) |= (1 << (x % 8));
      else
        *((unsigned char*) ((unsigned char*)pgdc->addr + y * ((pgdc->cx+7)/8) + x/8)) &= ~(1 << (x % 8));
#else
      unsigned char b;
      b = *((unsigned char*) ((unsigned char*)pgdc->addr + y * ((pgdc->cx+7)/8) + x/8));
      if (pgdc->fgcolor)
        b |=  (1 << (x % 8));
      else
        b &= ~(1 << (x % 8));
      *((unsigned char*) ((unsigned char*)pgdc->addr + y * ((pgdc->cx+7)/8) + x/8)) = b;
#endif
      break;
    case 8:
      *((unsigned char*) ((unsigned char*)pgdc->addr + x + y * pgdc->xres)) = (unsigned char) pgdc->fgcolor;
      break;
    case 16:
      *((unsigned short*) ((unsigned char*)pgdc->addr + x * 2 + y * pgdc->xres * 2)) = (unsigned short) pgdc->fgcolor;
      break;
#if defined (FEATURE_BPP32)
    case 32:
      *((unsigned long*) ((unsigned char*)pgdc->addr + x * 4 + y * pgdc->xres * 4)) = (unsigned long) pgdc->fgcolor;
      break;
#endif // FEATURE_BPP32
    default:
      error("%s: %d not supported\n", __FUNCTION__, pgdc->bpp);
  }

  return;
}

static void hLine(int fd, int x0, int y0, int x1)
{
  int bak;
  PGDC pgdc = (PGDC) fd;
  int sep = 0;

 // printf("hLine: %d %d %d\n", x0, y0, x1);

  // x and y have been already fit to dc
  if (x0 > x1)
  {
    bak = x1;
    x1 = x0;
    x0 = bak;
  }
  lcd_point(fd, x0, y0);
  x0++;

  if (x0 >= x1) return;

  while (x0 != x1)
  {
    if (pgdc->ps == PS_DOT)
    {
      if ((sep & 0x3) == 0x3)
        lcd_point(fd, x0, y0);
      sep ++;
    }
    else
    if (pgdc->ps == PS_SOLID)
    {
      lcd_point(fd, x0, y0);
    }
    x0 ++;
  }
  return;
}

static void vLine(int fd, int x0, int y0, int y1)
{
  int bak;
//  printf("vLine: %d %d %d\n", x0, y0, y1);

  if (((PGDC) fd)->ps != PS_SOLID)
  {
    error("Only PS_SOLID is supported in %s", __FUNCTION__);
  }

  // x and y have been already fit to dc
  if(y0 > y1)
  {
    bak = y1;
    y1 = y0;
    y0 = bak;
  }

  while(y1 >= y0)
  {
    lcd_point(fd, x0, y0);
    y0++;
  }
  return;
}

void grph_line(int fd, int x0, int y0, int x1, int y1)
{
  short dx = 0, dy = 0;
  signed char  dx_sym = 0, dy_sym = 0;
  short dx_x2 = 0, dy_x2 = 0;
  short di = 0;
 // PGDC pgdc = (PGDC) fd;

//printf("line: %d %d %d\n", pgdc->x, pgdc->cx, pgdc->xres);
/*  x0 += pgdc->x;
  y0 += pgdc->y;
  x1 += pgdc->x;
  y1 += pgdc->y;
*/
  dx = x1-x0;
  dy = y1-y0;

/*  x += pgdc->x;
  y += pgdc->y;

  if (cx<0) cx = 0;
  if (cy<0) cy = 0;

  if (x > pgdc->x+pgdc->cx) return;
  else if (x+cx > pgdc->x+pgdc->cx) cx = pgdc->x+pgdc->cx -1 - x;

  if (y > pgdc->y+pgdc->cy) return;
  else if (y+cy > pgdc->y+pgdc->cy) cy = pgdc->y+pgdc->cy -1 - y;
*/

  if(dx == 0)          /* vertical line */
  {
   /* if (x0 < pgdc->x || x0 > pgdc->x + pgdc->cx) return;
    y0 = (y0 < pgdc->y) ? pgdc->y : y0;
    y0 = (y0 >= pgdc->y+pgdc->cy) ? pgdc->y+pgdc->cy-1 : y0;
    y1 = (y1 < pgdc->y) ? pgdc->y : y1;
    y1 = (y1 >= pgdc->y+pgdc->cy) ? pgdc->y+pgdc->cy-1 : y1;*/
    vLine(fd, x0, y0, y1);
    return;
  }

  if(dx > 0)
  {
    dx_sym = 1;
  }
  else
  {
    dx_sym = -1;
  }

  if(dy == 0)          /* horizontal line */
  {
   /* if (y0 < pgdc->y || y0 > pgdc->y + pgdc->cy) return;
printf("grph_line %d %d  %d %d\n", x0, x1, pgdc->x, pgdc->cx);
    x0 = (x0 < pgdc->x) ? pgdc->x : x0;
    x0 = (x0 >= pgdc->x+pgdc->cx) ? pgdc->x+pgdc->cx-1 : x0;
    x1 = (x1 < pgdc->x) ? pgdc->x : x1;
    x1 = (x1 >= pgdc->x+pgdc->cx) ? pgdc->x+pgdc->cx-1 : x1;*/
    hLine(fd, x0, y0, x1);
    return;
  }

  if (((PGDC) fd)->ps != PS_SOLID)
  {
    error("Only PS_SOLID is supported in %s", __FUNCTION__);
  }


  if(dy > 0)
  {
    dy_sym = 1;
  }
  else
  {
    dy_sym = -1;
  }

  dx = dx_sym*dx;
  dy = dy_sym*dy;

  dx_x2 = dx*2;
  dy_x2 = dy*2;

  if(dx >= dy)
  {
    di = dy_x2 - dx;
    while(x0 != x1)
    {
      lcd_point(fd, x0, y0);
      x0 += dx_sym;
      if(di<0)
      {
        di += dy_x2;
      }
      else
      {
        di += dy_x2 - dx_x2;
        y0 += dy_sym;
      }
     // if (x0<0 || x0>=pgdc->cx) break;
     // if (y0<0 || y0>=pgdc->cy) break;
    }
    lcd_point(fd, x0, y0);
  }
  else
  {
    di = dx_x2 - dy;
    while(y0 != y1)
    {
      lcd_point(fd, x0, y0);
      y0 += dy_sym;
      if(di < 0)
      {
        di += dx_x2;
      }
      else
      {
        di += dx_x2 - dy_x2;
        x0 += dx_sym;
      }
     // if (x0<0 || x0>=pgdc->cx) break;
     // if (y0<0 || y0>=pgdc->cy) break;
    }
    lcd_point(fd, x0, y0);
  }
  return;
}

void grph_line_bound(int fd, int x0, int y0, int x1, int y1, int yl, int yu)
{
  short dx = 0, dy = 0;
  signed char  dx_sym = 0, dy_sym = 0;
  short dx_x2 = 0, dy_x2 = 0;
  short di = 0;
 // PGDC pgdc = (PGDC) fd;

//printf("line: %d %d %d\n", pgdc->x, pgdc->cx, pgdc->xres);
  dx = x1-x0;
  dy = y1-y0;


  if(dx == 0)          /* vertical line */
  {
    if (y0 < yl && y1 < yl) return;
    if (y0 > yu && y1 > yu) return;
    if (y0 < yl) y0 = yl;
    if (y0 > yu) y0 = yu;
    if (y1 < yl) y1 = yl;
    if (y1 > yu) y1 = yu;
    vLine(fd, x0, y0, y1);
    return;
  }

  if(dx > 0)
  {
    dx_sym = 1;
  }
  else
  {
    dx_sym = -1;
  }

  if(dy == 0)          /* horizontal line */
  {
    if (y0 < yl) return;
    if (y0 > yu) return;
    hLine(fd, x0, y0, x1);
    return;
  }

  if (((PGDC) fd)->ps != PS_SOLID)
  {
    error("Only PS_SOLID is supported in %s", __FUNCTION__);
  }


  if(dy > 0)
  {
    dy_sym = 1;
  }
  else
  {
    dy_sym = -1;
  }

  dx = dx_sym*dx;
  dy = dy_sym*dy;

  dx_x2 = dx*2;
  dy_x2 = dy*2;

  if(dx >= dy)
  {
    di = dy_x2 - dx;
    while(x0 != x1)
    {
      if (y0 >= yl && y0 <= yu)
      {
        lcd_point(fd, x0, y0);
      }
      x0 += dx_sym;
      if(di<0)
      {
        di += dy_x2;
      }
      else
      {
        di += dy_x2 - dx_x2;
        y0 += dy_sym;
      }
    }
    if (y0 >= yl && y0 <= yu)
    {
      lcd_point(fd, x0, y0);
    }
  }
  else
  {
    di = dx_x2 - dy;
    while(y0 != y1)
    {
      if (y0 >= yl && y0 <= yu)
      {
        lcd_point(fd, x0, y0);
      }
      y0 += dy_sym;
      if(di < 0)
      {
        di += dx_x2;
      }
      else
      {
        di += dx_x2 - dy_x2;
        x0 += dx_sym;
      }
    }
    if (y0 >= yl && y0 <= yu)
    {
      lcd_point(fd, x0, y0);
    }
  }
  return;
}

void grph_filldata(int fd, int x, int y, int cx, int cy, void * prgb)
{
}

void grph_fillrect(int fd, int x, int y, int cx, int cy, unsigned int RGB)
{
  int xx, yy;
  PGDC pgdc = (PGDC) fd;
  int bypp;

  if (cx<0) cx = 0;
  if (cy<0) cy = 0;

  if (x > pgdc->x+pgdc->cx) return;
  else if (x+cx > pgdc->x+pgdc->cx) cx = pgdc->x+pgdc->cx -1 - x;

  if (y > pgdc->y+pgdc->cy) return;
  else if (y+cy > pgdc->y+pgdc->cy) cy = pgdc->y+pgdc->cy -1 - y;

  assert(pgdc->addr != 0);
  assert(cx*((pgdc->bpp+7)/8) < sizeof(line));

  if (pgdc->bpp == 1)
  {
    unsigned char b;
    b = (RGB&0x1) ? 0xff : 0x00;
    line[0] = (b << (x%8)) & 0xff;
    b = (RGB&0x1) ? 0xff : 0x00;
    line[(int)((cx+7)/8)] = b >> (8-((x+cx)&7));
    memset(line+1, (RGB&0x1) ? 0xff : 0x00, (int)((cx-((8-x)&0x7))/8));
  }
  else
  if (pgdc->bpp == 8)
  {
    for(xx=0; xx<cx; xx++)
    {
      *((unsigned char*)line + xx) = RGB & 0xFF;
    }
  }
  else
  if (pgdc->bpp == 16)
  {
    for(xx=0; xx<cx; xx++)
    {
      *((unsigned short*)line + xx) = COLOR2PIXEL565(RGB);
    }
  }
#if defined (FEATURE_BPP32)
  else
  if (pgdc->bpp == 32)
  {
    for(xx=0; xx<cx; xx++)
    {
      *((unsigned long*)line + xx) = COLOR2PIXEL8888(RGB);
    }
  }
#endif // FEATURE_BPP32
  else
  {
    error("%s: %d bpp not supported\n", __FUNCTION__, pgdc->bpp);
  }

  if (pgdc->bpp == 1)
  {
    for (yy=y; yy<y+cy; yy++)
    {
      memcpy((unsigned char*)pgdc->addr + yy * ((pgdc->xres+7)/8) + x / 8, line, ((x&0x7)?1:0) + ((x+cx)&0x7?1:0) + (int)((cx-((8-x)&0x7))/8) );
    }
  }
  else
  if (pgdc->bpp == 8 || pgdc->bpp == 16)
  {
 // memsetw(line, cx, COLOR2PIXEL565(RGB));
    bypp = (int)((pgdc->bpp+7) / 8);
    for (yy=y; yy<y+cy; yy++)
    {
      memcpy((unsigned char*)pgdc->addr + x * bypp + yy * pgdc->xres * bypp, (unsigned char*)line, cx*bypp);
    }
  }
#if defined (FEATURE_BPP32)
  else
  if (pgdc->bpp == 32)
  {
 // memsetw(line, cx, COLOR2PIXEL565(RGB));
    bypp = (int)((pgdc->bpp+7) / 8);
    for (yy=y; yy<y+cy; yy++)
    {
      memcpy((unsigned char*)pgdc->addr + x * bypp + yy * pgdc->xres * bypp, (unsigned char*)line, cx*bypp);
    }
  }
#endif
  else
  {
    error("%s: %d bpp not supported\n", __FUNCTION__, pgdc->bpp);
  }
}

#define RR_CX        4
#define RR_CY        4
static unsigned char rounded_mask[] = 
{
  0x0F, 0x03, 0x01, 0x01,
};
void grph_fillroundedrect(int fd, int x, int y, int cx, int cy, unsigned int RGB)
{
  unsigned long fgcolor_old;

  if (!fd) return;

  grph_fillrect(fd, x, y, cx, cy, RGB);

  fgcolor_old = ((PGDC)(fd))->fgcolor;
  grph_setfgcolor(fd, 0x000000);
  int i,j;
  for (i=0; i<RR_CY; i++)
  {
    for (j=0; j<RR_CX; j++)
    {
      if (rounded_mask[i*((RR_CX+7)/8) + j/8] & (1<<(j&0x7)))
      {
        grph_line(fd, x+j, y+i, x+j, y+i);
        grph_line(fd, x+j, y+cy-1-i, x+j, y+cy-1-i);
        grph_line(fd, x+cx-1-j, y+i, x+cx-1-j, y+i);
        grph_line(fd, x+cx-1-j, y+cy-1-i, x+cx-1-j, y+cy-1-i);
      }
    }
  }
  ((PGDC)(fd))->fgcolor = fgcolor_old;
}

void grph_bitblt(int fd_to, int x, int y, int cx, int cy, int fd_from, int x1, int y1, int rop)
{
  int xx, yy;
  PGDC pgdc_to = (PGDC) fd_to;
  PGDC pgdc_from = (PGDC) fd_from;
  int bypp;

  if (x < 0) return;
  if (y < 0) return;

  assert(pgdc_from->bpp == pgdc_to->bpp);
  bypp = (int)((pgdc_to->bpp + 7) / 8);

#if !defined (FEATURE_BPP32)
  if (pgdc_to->bpp != 16)
  {
    error("%s: check support %d bpp\n", pgdc_to->bpp);
  }
#endif

#if defined (FEATURE_BPP32)
  if (pgdc_to->bpp == 32)
  {
    for (yy=0; yy<cy; yy++)
    {
      if (rop == GRPH_SRCPAINT)
      {
        for (xx=0; xx<cx; xx++)
        {
          *((unsigned long*)((unsigned char*)pgdc_from->addr + (y1 + yy) * pgdc_from->xres * bypp +  (x1+xx) * bypp)) |= *((unsigned long*)((unsigned char*)pgdc_to->addr + (y + yy) * pgdc_to->xres * bypp + (x+xx) * bypp));
        }
      }

      memcpy((unsigned char*)pgdc_to->addr + (y + yy) * pgdc_to->xres * bypp + x * bypp, 
             (unsigned char*)pgdc_from->addr + (y1 + yy) * pgdc_from->xres * bypp +  x1 * bypp,
             bypp*cx);
    }
    return;
  }
#endif

  for (yy=0; yy<cy; yy++)
  {
    if (rop == GRPH_SRCPAINT)
    {
      for (xx=0; xx<cx; xx++)
      {
        *((unsigned short*)((unsigned char*)pgdc_from->addr + (y1 + yy) * pgdc_from->xres * bypp +  (x1+xx) * bypp)) |= *((unsigned short*)((unsigned char*)pgdc_to->addr + (y + yy) * pgdc_to->xres * bypp + (x+xx) * bypp));
      }
    }

    memcpy((unsigned char*)pgdc_to->addr + (y + yy) * pgdc_to->xres * bypp + x * bypp, 
           (unsigned char*)pgdc_from->addr + (y1 + yy) * pgdc_from->xres * bypp +  x1 * bypp,
           bypp*cx);
  }
}

int grph_createdc(int fd, int x, int y, int cx, int cy, int flags)
{
  PGDC pgdc = (PGDC) fd;
  PGDC pgdc_new = calloc(1, sizeof(gdc_t));
  int bypp;

  assert(pgdc_new);
  memcpy(pgdc_new, pgdc, sizeof(gdc_t));
  pgdc_new->x  = (x>0)?x:0;
  pgdc_new->y  = (y>0)?y:0;
  pgdc_new->cx = (cx>0)?cx:0;
  pgdc_new->cy = (cy>0)?cy:0;

  if (pgdc_new->x >= pgdc->x+pgdc->cx) pgdc_new->x = pgdc->x+pgdc->cx -1;
  if (pgdc_new->y >= pgdc->y+pgdc->cy) pgdc_new->y = pgdc->y+pgdc->cy -1;

  if (pgdc_new->x + pgdc_new->cx >= pgdc->x+pgdc->cx) pgdc_new->cx = pgdc->x+pgdc->cx - 1 - pgdc_new->x;
  if (pgdc_new->y + pgdc_new->cy >= pgdc->y+pgdc->cy) pgdc_new->cy = pgdc->y+pgdc->cy - 1 - pgdc_new->y;

  pgdc_new->xres = pgdc_new->cx;
  pgdc_new->yres = pgdc_new->cy;

 // printf("grph_createdc: %d %d %d %d\n", pgdc_new->x, pgdc_new->y, pgdc_new->cx, pgdc_new->cy);

  if (pgdc->bpp == 8 || pgdc->bpp == 16)
    bypp = (int)((pgdc->bpp+7) / 8);
#if defined (FEATURE_BPP32)
  else
  if (pgdc->bpp == 32)
    bypp = (int)((pgdc->bpp+7) / 8);
#endif
  else
  {
    error("%s: check support %d bpp\n", __FUNCTION__, pgdc->bpp);
    return 0;
  }

  pgdc_new->addr = calloc(bypp, cx*cy);
  assert(pgdc_new->addr);

  return (int) pgdc_new;
}

void grph_releasedc(int fd)
{
  PGDC pgdc = (PGDC) fd;
  if (!pgdc) return;
  free(pgdc->addr);
  free(pgdc);
}

void grph_changedc(int fd, PGSI pgsi)
{
  PGDC pgdc = (PGDC) fd;
  if (!pgdc) return;
  if (!pgsi) return;
  assert(pgdc->xres*pgdc->yres*((pgdc->bpp)+7/8) >= pgsi->xres*pgsi->yres*((pgdc->bpp)+7/8));
  free(pgdc->addr);
  pgdc->xres = pgdc->cx = pgsi->xres;
  pgdc->yres = pgdc->cy = pgsi->yres;
  pgdc->addr = pgsi->addr;
}

static int lcd_putchar(PGDC pgdc, unsigned short x, unsigned short y, char ch, unsigned long bkcolor, unsigned long fgcolor, unsigned int format, unsigned int * pwn)
{
  unsigned char data = 0;
  unsigned char i = 0, j = 0;
  unsigned char b;
  ykf_charinfo_t ci;
  int font_h;

  b = ch;
#if 1
  if( (b < 0x20)/* || (ch > 0x7f)*/ )
  {
    b = 0x20;     /* unknown character will be set to blank */
  }
#endif
  b -= 0x20;

  ykf_get_charinfo(fnt[pgdc->font_id], b, &ci);

  if (format == DT_CALCRECT)
  {
    if (pwn) *pwn = ci.w;
    return (1);
  }

  font_h = ykf_get_font_h(fnt[pgdc->font_id]);
  for(i=0; i<font_h; i++)
  {
    int k;
    data = 0;
    for (k=0; k<ci.nblocks; k++)
    {
      unsigned short offset;
      offset = i * ci.nblocks + k;
      data = ci.data[offset];

//printf("%c %x: %X\n", b+0x20, b+0x20, data);

      for(j=0; j<8; j++)
      {
        if( (data & bitmask[j]) == 0 )
        {
         // color = bkcolor;
          pgdc->fgcolor = bkcolor;
        }
        else
        {
         // color = fgcolor;
          pgdc->fgcolor = fgcolor;
          lcd_point((int)pgdc, x, y);
        }
       // lcd_point((int)pgdc, x, y);
        x++;
      }
    }
    y++;
    x -= 8*ci.nblocks;
  }
  if (pwn) *pwn = ci.w;

  return( 1 );
}

static int lcd_putstring(PGDC pgdc, unsigned short x, unsigned short y, char *pStr, int len, unsigned int format)
{
  int i;
  unsigned long bkcolor, fgcolor;
  char ch;
  unsigned char b;
  unsigned int wn;
  unsigned short x0;

  x0 = x;
  bkcolor = pgdc->bkcolor;
  fgcolor = pgdc->fgcolor;

  for (i=0; i<(unsigned int)len; i++)
  {
    b = *pStr;
    if( b =='\0' )
    {
      break;
    }
   /* if (b < 0x80)
    {
      ch = b;
      pStr ++;
    }
    else
    {
      unsigned short w;
      b = *pStr;
      w = (b << 8);
      pStr ++;
      b = *pStr;
      w |= b;
      pStr ++;
      if (w & 0x0100)
        ch = (0xF0 + w - 0xD180);
      else
        ch = (0xC0 + w - 0xD090);
    }*/
    ch = b;
    pStr ++;
    if( lcd_putchar(pgdc, x, y, ch, bkcolor, fgcolor, format, &wn) == 0 )
    {
      break;
    }
    if (wn)
      x += (wn + gui_fonts[pgdc->font_id].dx);
   // else
   //   x += (gui_fonts[pgdc->font_id].w + gui_fonts[pgdc->font_id].dx);
  }

  pgdc->bkcolor = bkcolor;
  pgdc->fgcolor = fgcolor;

  if (format == DT_CALCRECT)
  {
    return (x - x0);
  }

  return 1;
}

static int lcd_putchar_vert(PGDC pgdc, unsigned short x, unsigned short y, char ch, unsigned long bkcolor, unsigned long fgcolor, unsigned int format, unsigned int * pwn)
{
  unsigned char data = 0;
  unsigned char i = 0, j = 0;
  ykf_charinfo_t ci;
  int font_h;
  unsigned char b;

  b = ch;
#if 1
  if( (b < 0x20)/* || (ch > 0x7f)*/ )
  {
    b = 0x20;     /* unknown character will be set to blank */
  }
#endif
  b -= 0x20;

  ykf_get_charinfo(fnt[pgdc->font_id], b, &ci);

  if (format == DT_CALCRECT)
  {
    if (pwn) *pwn = ci.h;
    return (1);
  }

  font_h = ykf_get_font_h(fnt[pgdc->font_id]);
  for(i=0; i<font_h; i++)
  {
    int k;
    data = 0;
    for (k=0; k<ci.nblocks; k++)
    {
      unsigned short offset;
      offset = i * (ci.nblocks) + k;
      data = ci.data[offset];

//printf("%c %x: %X\n", b+0x20, b+0x20, data);

      for(j=0; j<8; j++)
      {
        if( (data & bitmask[j]) == 0 )
        {
         // color = bkcolor;
          pgdc->fgcolor = bkcolor;
        }
        else
        {
         // color = fgcolor;
          pgdc->fgcolor = fgcolor;
          lcd_point((int)pgdc, x, y);
        }
       // lcd_point((int)pgdc, x, y);
        y--;
      }
    }
    x++;
    y += 8*ci.nblocks;
  }
  if (pwn) *pwn = ci.w;
  return( 1 );
}

/*static */int lcd_putstring_vert(PGDC pgdc, unsigned short x, unsigned short y, char *pStr, int len, unsigned int format)
{
  int i;
  unsigned long bkcolor, fgcolor;
  char ch;
  unsigned char b;
  unsigned int wn;
  unsigned short y0;

  y0 = y;
  bkcolor = pgdc->bkcolor;
  fgcolor = pgdc->fgcolor;

  for (i=0; i<(unsigned int)len; i++)
  {
    b = *pStr;
    if( b =='\0' )
    {
      break;
    }
   /* if (b < 0x80)
    {
      ch = b;
      pStr ++;
    }
    else
    {
      unsigned short w;
      b = *pStr;
      w = (b << 8);
      pStr ++;
      b = *pStr;
      w |= b;
      pStr ++;
      if (w & 0x0100)
        ch = (0xF0 + w - 0xD180);
      else
        ch = (0xC0 + w - 0xD090);
    }
   */
    ch = b;
    pStr ++;
    if( lcd_putchar_vert(pgdc, x, y, ch, bkcolor, fgcolor, format, &wn) == 0 )
    {
      break;
    }
    if (wn)
      y -= (wn + gui_fonts[pgdc->font_id].dx);
   // else
   //   y -= (gui_fonts[pgdc->font_id].w + gui_fonts[pgdc->font_id].dx);
  }

  pgdc->bkcolor = bkcolor;
  pgdc->fgcolor = fgcolor;

  if (format == DT_CALCRECT)
  {
    return (y0 - y);
  }

  return 1;
}

void grph_textout(int fd, int x, int y, char * s, int len)
{
  PGDC pgdc = (PGDC) fd;
  lcd_putstring(pgdc, x, y, s, len, 0);
}

void grph_drawtext(int fd, char * ps, int count, rect_t *lrc, unsigned int format)
{
  PGDC pgdc = (PGDC) fd;
  int len;
  int x0, y0, x1=0, y1=0;
  unsigned short cw;

  assert(lrc);

  x0 = lrc->x0;
  y0 = lrc->y0;

  len = (count < 0) ? strlen(ps) : count;

  if (format & DT_CALCRECT || format & DT_CENTER || format & DT_RIGHT)
  {
    if (format & DT_VERTICAL)
    {
      cw = lcd_putstring(pgdc, x0, y0, ps, len, DT_CALCRECT);
      y1 = lrc->y0 - cw;
      if (format & DT_CALCRECT)
      {
        // swap y1,y0
       // lrc->y1 = y0;
       // lrc->y0 = y1;
        lrc->y1 = y1;
        lrc->x1 = lrc->x0 + ykf_get_font_h(fnt[pgdc->font_id]); //gui_fonts[pgdc->font_id].h;
        return;
      }
      if (lrc->y0 < lrc->y1)
      {
        int t;
        t = lrc->y0;
        lrc->y0 = lrc->y1;
        lrc->y1 = t;
      }
      y0 -= ((lrc->y0 - lrc->y1) - (y0 - y1))/2;
    }
    else
    {
      cw = lcd_putstring(pgdc, x0, y0, ps, len, DT_CALCRECT);
      if (format & DT_CENTER || format & DT_CALCRECT)
      {
        x1 = lrc->x0 + cw;
      }
      if (format & DT_RIGHT)
      {
        x0 = lrc->x1 - cw;
      }
      if (format & DT_CALCRECT)
      {
        lrc->x1 = x1;
        lrc->y1 = lrc->y0 + ykf_get_font_h(fnt[pgdc->font_id]);//gui_fonts[pgdc->font_id].h;
        return;
      }
      if (format & DT_CENTER)
      {
        x0 += ((lrc->x1 - lrc->x0) - (x1 - x0))/2;
      }
    }
  }

  if (format & DT_VERTICAL)
    lcd_putstring_vert(pgdc, x0, y0, ps, len, format);
  else
    lcd_putstring(pgdc, x0, y0, ps, len, format);
}

void grph_circle(int fd, short x0, short y0, short r)
{
  PGDC pgdc = (PGDC) fd;
  signed short draw_x0, draw_y0;
  signed short draw_x1, draw_y1;
  signed short draw_x2, draw_y2;
  signed short draw_x3, draw_y3;
  signed short draw_x4, draw_y4;
  signed short draw_x5, draw_y5;
  signed short draw_x6, draw_y6;
  signed short draw_x7, draw_y7;
  signed short xx, yy;
  signed short di;

  if(r == 0)               /* no radius */
  {
    return;
  }

  draw_x0 = draw_x1 = x0;
  draw_y0 = draw_y1 = y0 + r;
  if(draw_y0 < pgdc->yres)
  {
    lcd_point(fd, draw_x0, draw_y0);	   /* 90 degree */
  }

  draw_x2 = draw_x3 = x0;
  draw_y2 = draw_y3 = y0 - r;
  if(draw_y2 >= 0)
  {
    lcd_point(fd, draw_x2, draw_y2);    /* 270 degree */
  }

  draw_x4 = draw_x6 = x0 + r;
  draw_y4 = draw_y6 = y0;
  if(draw_x4 < pgdc->xres)
  {
    lcd_point(fd, draw_x4, draw_y4);	   /* 0 degree */
  }

  draw_x5 = draw_x7 = x0 - r;
  draw_y5 = draw_y7 = y0;
  if(draw_x5>=0)
  {
    lcd_point(fd, draw_x5, draw_y5);	   /* 180 degree */
  }

  if(r == 1)
  {
    return;
  }

  di = 3 - 2*r;
  xx = 0;
  yy = r;
  while(xx < yy)
  {
    if(di < 0)
    {
      di += 4*xx + 6;
    }
    else
    {
      di += 4*(xx - yy) + 10;
      yy--;
      draw_y0--;
      draw_y1--;
      draw_y2++;
      draw_y3++;
      draw_x4--;
      draw_x5++;
      draw_x6--;
      draw_x7++;
    }
    xx++;
    draw_x0++;
    draw_x1--;
    draw_x2++;
    draw_x3--;
    draw_y4++;
    draw_y5++;
    draw_y6--;
    draw_y7--;

    if( (draw_x0 <= pgdc->xres) && (draw_y0>=0) )
    {
      lcd_point(fd, draw_x0, draw_y0);
    }

    if( (draw_x1 >= 0) && (draw_y1 >= 0) )
    {
      lcd_point(fd, draw_x1, draw_y1);
    }

    if( (draw_x2 <= pgdc->xres) && (draw_y2 <= pgdc->yres) )
    {
      lcd_point(fd, draw_x2, draw_y2);
    }

    if( (draw_x3 >=0 ) && (draw_y3 <= pgdc->yres) )
    {
      lcd_point(fd, draw_x3, draw_y3);
    }

    if( (draw_x4 <= pgdc->yres) && (draw_y4 >= 0) )
    {
      lcd_point(fd, draw_x4, draw_y4);
    }

    if( (draw_x5 >= 0) && (draw_y5 >= 0) )
    {
      lcd_point(fd, draw_x5, draw_y5);
    }
    if( (draw_x6 <= pgdc->xres) && (draw_y6 <= pgdc->yres) )
    {
      lcd_point(fd, draw_x6, draw_y6);
    }
    if( (draw_x7 >= 0) && (draw_y7 <= pgdc->yres) )
    {
      lcd_point(fd, draw_x7, draw_y7);
    }
  }
  return;
}

void grph_filldata16(int fd, int x, int y, int cx, int cy, void * data)
{
  int yy;
  PGDC pgdc = (PGDC) fd;

#if defined (FEATURE_BPP32)
  if (pgdc->bpp == 32)
  {
    return grph_filldata32(fd, x, y, cx, cy, data);
  }
#endif

  if (cx<0) cx = 0;
  if (cy<0) cy = 0;

  if (x > pgdc->x+pgdc->cx) return;
  else if (x+cx > pgdc->x+pgdc->cx) cx = pgdc->x+pgdc->cx -1 - x;

  if (y > pgdc->y+pgdc->cy) return;
  else if (y+cy > pgdc->y+pgdc->cy) cy = pgdc->y+pgdc->cy -1 - y;

  assert(pgdc->addr);
  assert(pgdc->bpp == 16);

  for (yy=0; yy<cy; yy++)
  {
    memcpy((unsigned char*)pgdc->addr + x * 2 + (y+yy) * pgdc->xres * 2, (unsigned char*)data + yy*cx*2, cx*2); // 2 - bypp
  }
}

static void grph_filldata32(int fd, int x, int y, int cx, int cy, void * data)
{
  int yy;
  PGDC pgdc = (PGDC) fd;

  if (cx<0) cx = 0;
  if (cy<0) cy = 0;

  if (x > pgdc->x+pgdc->cx) return;
  else if (x+cx > pgdc->x+pgdc->cx) cx = pgdc->x+pgdc->cx -1 - x;

  if (y > pgdc->y+pgdc->cy) return;
  else if (y+cy > pgdc->y+pgdc->cy) cy = pgdc->y+pgdc->cy -1 - y;

  assert(pgdc->addr);
  assert(pgdc->bpp == 32);

  for (yy=0; yy<cy; yy++)
  {
    unsigned long val;
    unsigned short w;
    int xx;
    for (xx=0; xx<cx; xx++)
    {
      w = *((unsigned short*)((unsigned char*)data + cx*yy*2 + xx*2));
      val = PIXEL565TOCOLORVAL(w);
      *((unsigned long*)((unsigned char*)pgdc->addr + (x+xx) * 4 + (y+yy) * pgdc->xres * 4)) = val;
    }
  }
}

void grph_filldatastretch16(int fd, int x, int y, int cx_src, int cy_src, int cx_dst, int cy_dst, void * data)
{
  int i, yy;

  if (cx_src == cx_dst && cy_src == cy_dst)
  {
#if defined (FEATURE_BPP32)
    PGDC pgdc = (PGDC) fd;
    if (pgdc->bpp == 32)
      return grph_filldata32(fd, x, y, cx_src, cy_src, data);
#else
    return grph_filldata16(fd, x, y, cx_src, cy_src, data);
#endif
  }

#if 1
 // line = malloc(cx_dst * 2);
  assert(cx_dst * 2 < sizeof(line));
  if (cx_src > cx_dst && cy_src > cy_dst)
  {
    yy = 0;
    for (i=0; i<cy_dst; i++)
    {
      int k;
      for (k=0; k<cx_dst; k++)
      {
        *((unsigned short*)line + k) = *((unsigned short*)data + (i*cy_src/cy_dst)*cx_src + k*cx_src/cx_dst);
      }
#if defined (FEATURE_BPP32)
      PGDC pgdc = (PGDC) fd;
      if (pgdc->bpp == 32)
        grph_filldata32(fd, x, y+yy, cx_dst, 1, line);
#else
      grph_filldata16(fd, x, y+yy, cx_dst, 1, line);
#endif
      yy ++;
    }
  }
#else
#if 0
  unsigned char * data_dst;
  unsigned short w;
  int n, k;
  data_dst = malloc(cx_dst * cy_dst * 2);

  n = 0;
  for (yy=0; yy<cy_dst; yy++)
  {
    for (i=0; i<cx_dst; i++)
    {
      n = cx_src/cx_dst;
      w = 0;
      for (k=0; k<n; k++)
        w += *((unsigned short*)data + yy*(cy_src/cy_dst)*cx_src + i*n + k);
      w /= n;
/*
      k = 0;
      w = *((unsigned short*)data + yy*(cy_src/cy_dst)*cx_src + i*n + k);*/
      *((unsigned short*)data_dst + yy*cx_dst+i) = w;
    }
  }
  grph_filldata16(fd, x, y, cx_dst, cy_dst, data_dst);
  free(data_dst);
#endif
#endif
}

#define fixed int
#define round(x) floor(x + 0.5)
#define roundf(x) floor(x + 0.5f)

// ������������ ����� ����� � ������� ����� � ������������� ������
inline fixed int_to_fixed(int value)
{
  return (value << 16);
}

// ����� ����� ����� � ������������� ������
inline int fixed_to_int(fixed value)
{
  if (value < 0) return ((value >> 16) - 1);
  /*if (value >= 0) */ else return (value >> 16);
}

// ���������� �� ���������� ������
inline int round_fixed(fixed value)
{
  return fixed_to_int(value + (1 << 15));
}

// ������������ ����� � ��������� ������ � ������� ����� � ������������� ������
// ����� ���������� ������� ������ ��������
inline fixed double_to_fixed(double value)
{
  return round(value * (65536.0));
}

inline fixed float_to_fixed(float value)
{
  return roundf(value * (65536.0f));
}

// ���������� ��������� (a / b) � ������� ����� � ������������� ������
inline fixed frac_to_fixed(int a, int b)
{
  return (a << 16) / b;
}

inline void swap(int *a, int *b)
{
  int t;
  t = *a;
  *a = *b;
  *b = t;
}

void grph_filltriangle(int fd, int x1, int y1, int x2, int y2, int x3, int y3)
{
  int i, j;
	// ������������� ����� p1(x1, y1),
	// p2(x2, y2), p3(x3, y3)
	if (y2 < y1) {
		swap(&y1, &y2);
		swap(&x1, &x2);
	} // ����� p1, p2 �����������
	if (y3 < y1) {
		swap(&y1, &y3);
		swap(&x1, &x3);
	} // ����� p1, p3 �����������
	// ������ p1 ����� �������
	// �������� ����������� p2 � p3
	if (y2 > y3) {
		swap(&y2, &y3);
		swap(&x2, &x3);
	}

	// ���������� �� ��� x ��� ���� ������
	// ������������
	fixed dx13 = 0, dx12 = 0, dx23 = 0;

	// ��������� ����������
	// � ������, ���� �������� ���� �����
	// ���������, ����������
	// ���������� ������� ����
	if (y3 != y1) {
		dx13 = int_to_fixed(x3 - x1);
		dx13 /= y3 - y1;
	}
	else
	{
		dx13 = 0;
	}

	if (y2 != y1) {
		dx12 = int_to_fixed(x2 - x1);
		dx12 /= (y2 - y1);
	}
	else
	{
		dx12 = 0;
	}

	if (y3 != y2) {
		dx23 = int_to_fixed(x3 - x2);
		dx23 /= (y3 - y2);
	}
	else
	{
		dx23 = 0;
	}

	// "������� �����"
	// ���������� ��� ��������� � ������� �����
	fixed wx1 = int_to_fixed(x1);
	fixed wx2 = wx1;

	// ��������� ���������� dx13 � ������ ����������
	int _dx13 = dx13;

	// ������������� ���������� ����� �������, �����
	// � �������� ������ ���������
	// ����� wx1 ���� ������ ����� wx2
	if (dx13 > dx12)
	{
		swap(&dx13, &dx12);
	}

	// ����������� ������� ���������������
	for (i = y1; i < y2; i++){
		// ������ �������������� ����� ����� �������� �������
		for (j = fixed_to_int(wx1); j <= fixed_to_int(wx2); j++){
			lcd_point(fd, j, i);
		}
		wx1 += dx13;
		wx2 += dx12;
	}

	// ����������� ������, ����� �������� ���������������� ���
	// ���� �������� ������� ����� �� ��� x,
	// �.�. ���������� ��� ���������
	if (y1 == y2){
		wx1 = int_to_fixed(x1);
		wx2 = int_to_fixed(x2);
	}

	// ������������� ����������
	// (���������� ����������� ����������)
	if (_dx13 < dx23)
	{
		swap(&_dx13, &dx23);
	}
	
	// ����������� ������ ���������������
	for (i = y2; i <= y3; i++){
		// ������ �������������� ����� ����� �������� �������
		for (j = fixed_to_int(wx1); j <= fixed_to_int(wx2); j++){
			lcd_point(fd, j, i);//SetPixel(hdc, j, i, 0);
		}
		wx1 += _dx13;
		wx2 += dx23;
	}
}
