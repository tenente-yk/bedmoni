/*! \file ykf.c
    \brief Functions to operate with YKF fonts
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ramfont.h"
#include "ykf.h"

int  ykf_open(char * fname)
{
  ykf_info_t * p = (ykf_info_t*)calloc(1, sizeof(ykf_info_t));
  assert(p);
  p->f = fopen(fname, "rb");
  if (p->f)
  {
#if defined (USE_RAMFONT)
    p->fd = ramfont_create(p->f);
    fclose(p->f);
#else
    unsigned long v;
    fseek(p->f, 4, SEEK_SET); // skip "YKF\0"
    fread(&v,1,4,p->f);
    // fill fields font_h, font_w etc..
    memcpy(&p->font_h, &v, 4);
#endif
  }
  else
  {
    free(p);
    return 0;
  }

/*
#if defined (USE_RAMFONT)
  printf("%s -> fd 0x%08X\n", fname, p->fd);
#else
  printf("%s -> f 0x%08X\n", fname, p->f);
#endif
*/

  return (int) (p);
}

void ykf_close(int fd)
{
  ykf_info_t * p = (ykf_info_t*)fd;
#if defined (USE_RAMFONT)
  ramfont_destroy(p->fd);
#else
  if (p->f) fclose(p->f);
#endif
  free(p);
}

int ykf_get_font_h(int fd)
{
  ykf_info_t * p = (ykf_info_t*)fd;
  if (!p) return 0;
#if defined (USE_RAMFONT)
  return ramfont_get_font_h(p->fd);
#else
  return ((ykf_info_t*)fd)->font_h;
#endif
}

int ykf_get_font_w(int fd)
{
  ykf_info_t * p = (ykf_info_t*)fd;
  if (!p) return 0;
#if defined (USE_RAMFONT)
  return ramfont_get_font_w(p->fd);
#else
  return ((ykf_info_t*)fd)->font_w;
#endif
}

int ykf_get_charinfo(int fd, int c, ykf_charinfo_t * pci)
{
  ykf_info_t * p = (ykf_info_t*)fd;
  unsigned long addr;
  int nr;
//printf("c=%d (%c)\n", (unsigned char)c, (unsigned char)c);
  assert(pci);
  if (!p)
  {
    pci->h = pci->w = pci->nblocks = 0;
    pci->size=0;
    return 0;
  }
  assert(p->f);

#if defined (USE_RAMFONT)
  return ramfont_get_charinfo(p->fd, c, pci);
#endif

#if 1
  char s[4];
  int r;
 // assert( fseek(p->f, 0, SEEK_SET) == 0 );
  fpos_t filepos;
  filepos.__pos = 0;
  fsetpos(p->f, &filepos);
  memset(s, 0x44, 4);
  nr = fread(s,1,4,p->f);
  if (s[0] == 0)
  {
 //   printf("sync\n");
 //   fsync(fileno(p->f));
    nr = fread(s,1,4,p->f);
    return -1;
  }
  assert(nr == 4);
  printf("0x%08X %c(%02X) %c(%02X) %c(%02X) %d\n", (unsigned int)(p->f), s[0], (unsigned char)s[0], s[1], (unsigned char)s[1], s[2], (unsigned char)s[2], nr);
  r = (strncmp(s, "YKF", 3) == 0);
  assert(r);
#endif

  assert(c < p->nsymbols);
  fseek(p->f, 8+4*c, SEEK_SET);
  nr = fread(&addr,1,4,p->f);
  assert(nr == 4);
  fseek(p->f, addr, SEEK_SET);
  nr = fread(&pci->h,1,1,p->f);
  assert(nr == 1);
  nr = fread(&pci->w,1,1,p->f);
  assert(nr == 1);
  nr = fread(&pci->nblocks,1,1,p->f);
  assert(nr == 1);
 // fread(&pci->size,1,2,p->f);
  unsigned char b1=0, b2=0;
  nr = fread(&b1,1,1,p->f);
  assert(nr == 1);
  nr = fread(&b2,1,1,p->f);
  assert(nr == 1);
  pci->size = b2*256 + b1;
#if 1
  if (!(pci->size < sizeof(pci->data)))
  {
    printf("%d\n", pci->nblocks);
    printf("%d  %x h %d w %d %d %d  %d %d\n", p->font_h, c, pci->h, pci->w, pci->size, sizeof(pci->data), b1 , b2);
    return -1;
  }
#endif
  assert(pci->size < sizeof(pci->data));
  nr = fread(&pci->data,1,pci->nblocks*p->font_h,p->f);
  assert(nr == pci->nblocks*p->font_h);
 // return (pci->nblocks*p->font_h);
  return pci->size;
}

