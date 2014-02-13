#include <stdio.h>
#include <assert.h>
#include "bedmoni.h"
#include "fb.h"
#include "fb2bmp.h"

void fb2bmp24(int x, int y, int cx, int cy, char * fname)
{
  FILE *f;
  fb_info_t fb_info;
  unsigned long v;
  unsigned short w;
  int l, c;

  if (fb_get_info(&fb_info) < 0)
  {
    error("%s: fb_get_info error\n", __FUNCTION__);
    return;
  }

  if ( !((fb_info.bpp == 16) || (fb_info.bpp == 32)) )
  {
    error("%s: unsupported bpp %d\n", __FUNCTION__, fb_info.bpp);
    return;
  }

 // if (x fb_info.xres);

  f = fopen(fname, "wb");
  assert(f);

  // file header
  w = 0x4d42;             // 'BM'
  fwrite(&w, 1, 2, f);
  v = 54 + (24/8)*cx*cy;  // len
  fwrite(&v, 1, 4, f);
  w = 0;                  // reserved
  fwrite(&w, 1, 2, f);
  w = 0;                  // reserved
  fwrite(&w, 1, 2, f);
  v = 54;                 // image offset
  fwrite(&v, 1, 4, f);
  // bitmap header
  v = 40;                 // bitmap header len
  fwrite(&v, 1, 4, f);
  v = cx;                 // image width
  fwrite(&v, 1, 4, f);
  v = cy;                 // image height
  fwrite(&v, 1, 4, f);
  w = 1;                  // number of planes
  fwrite(&w, 1, 2, f);
  w = 24;                 // bpp
  fwrite(&w, 1, 2, f);
  v = 0;                  // compression type
  fwrite(&v, 1, 4, f);
  v = 0;                  // no compression
  fwrite(&v, 1, 4, f);
  v = 0;                  // horizontal resolution
  fwrite(&v, 1, 4, f);
  v = 0;                  // vertical resolution
  fwrite(&v, 1, 4, f);
  v = 0;                  // number of colours
  fwrite(&v, 1, 4, f);
  v = 0;                  // number of general colours
  fwrite(&v, 1, 4, f);

  for (l=cy; l>0; l--)
  {
    for (c=0; c<cx; c++)
    {
      if (fb_info.bpp == 16)
      {
        w = *((unsigned short*)fb_info.addr + (y+l)*fb_info.xres + (x+c));
        v = PIXEL565TORGB(w);
      }
      if (fb_info.bpp == 32)
      {
        v = *((unsigned long*)fb_info.addr + (y+l)*fb_info.xres + (x+c));
      }
      fwrite(&v, 1, 3, f);
    }
    v = 0xffffffff;
    if (cx%4) fwrite(&v, 1, 4 - ((3*cx)%4), f);
  }

  fclose(f);
}

