#ifndef __FB_H
#define __FB_H

#pragma pack(1)

typedef struct
{
  int             xres;
  int             yres;
  int             bpp;
  unsigned char * addr;
} fb_info_t;

#pragma pack()

int  fb_open(char * fbdev);

void fb_close(void);

int  fb_get_info(fb_info_t * pi);

#endif // __FB_H
