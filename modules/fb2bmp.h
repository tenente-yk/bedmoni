#ifndef __FB2BMP_H
#define __FB2BMP_H

#ifndef PIXEL565TORGB
#define PIXEL565TORGB(p) \
(0xff000000ul | (((p) & 0xf800u) << 8) | (((p) & 0x07e0u) << 5) | (((p) & 0x1ful) << 3) | 0xff000000ul)
#endif

void fb2bmp24(int x, int y, int cx, int cy, char * fname);

#endif // __FB2BMP_H
