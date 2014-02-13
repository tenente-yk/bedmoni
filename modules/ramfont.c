#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ykf.h"
#include "ramfont.h"

int  ramfont_create(FILE * f)
{
  unsigned char *pb, *p;
  unsigned long sz;
  unsigned char buf[1000];
  int nr;

  fseek(f, 0, SEEK_END);
  sz = ftell(f);

  pb = calloc(1, sz);
  assert(pb);

  fseek(f, 0, SEEK_SET);
  p = pb;
  while (!feof(f))
  {
    nr = fread(buf, 1, sizeof(buf), f);
    if (nr <= 0) continue;
    memcpy(p, buf, nr);
    p += nr;
    if (feof(f)) break;
  }

  return (int)(pb);
}

void ramfont_destroy(int fd)
{
  if (fd) free((void*)fd);
}

int  ramfont_get_font_h(int fd)
{
  unsigned char * pb = (unsigned char*)fd;
  assert(pb);
  return (int)(pb[4]);
}

int  ramfont_get_font_w(int fd)
{
  unsigned char * pb = (unsigned char*)fd;
  assert(pb);
  return (int)(pb[5]);
}

int  ramfont_get_charinfo(int fd, int c, ykf_charinfo_t * pci)
{
  unsigned char * pb = (unsigned char*)fd;
  assert(pb);

#if 0
  char s[4];
  int r;
  memcpy(s, pb, 4);
  printf("0x%08X %c(%02X) %c(%02X) %c(%02X) %d\n", fd, s[0], s[0], s[1], s[1], s[2], s[2], 4);
  r = (strncmp(s, "YKF", 3) == 0);
  assert(r);
#endif

  int nsymbols, offs, font_h;
  unsigned long addr;
  nsymbols = pb[6];
  assert(c < nsymbols);
  offs = 8+4*c;
  addr = *((unsigned long*)&pb[offs]);
  pci->h = pb[addr++];
  pci->w = pb[addr++];
  pci->nblocks = pb[addr++];
  unsigned char b1=0, b2=0;
  b1 = pb[addr++];
  b2 = pb[addr++];
  pci->size = b2*256 + b1;
#if 0
  if (!(pci->size < sizeof(pci->data)))
  {
    printf("%d\n", pci->nblocks);
    printf("%d  %x h %d w %d %d %d  %d %d\n", p->font_h, c, pci->h, pci->w, pci->size, sizeof(pci->data), b1 , b2);
    return;
  }
#endif
  assert(pci->size < sizeof(pci->data));
  font_h = ramfont_get_font_h(fd);
  memcpy(&pci->data, &pb[addr], pci->nblocks*font_h);
  return pci->size;
}


