#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <assert.h>
#include "bedmoni.h"
#include "fb.h"

static struct fb_var_screeninfo vinfo;
static struct fb_fix_screeninfo finfo;
static long int screensize = 0;

int fbfd = 0;
static unsigned char *fb_addr = 0;

int fb_open(char *fbdev)
{
  fbfd = open(fbdev, O_RDWR);
  if (fbfd < 0)
  {
    error("fb %s open failed\n", fbdev);
    return -1;
  }
 // debug("The framebuffer device was opened successfully.\n");

  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
  {
    error("fb %s read finfo failed\n", fbdev);
    return -1;
  }

  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
  {
    error("fb %s read vinfo failed\n", fbdev);
    return -1;
  }

  assert(finfo.line_length >= (vinfo.bits_per_pixel + 7) / 8 * vinfo.xres);
  vinfo.xres = finfo.line_length / ((vinfo.bits_per_pixel + 7) / 8);

 // debug("%dx%d, %dbpp, %d linelen %d xoffs %d yoffs\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel, 
//          finfo.line_length, vinfo.xoffset, vinfo.yoffset);

  // Figure out the size of the screen in bytes
  screensize = vinfo.xres * vinfo.yres * ((vinfo.bits_per_pixel + 8 - 1) / 8);

  // Map the device to memory
  fb_addr = (unsigned char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
  if ((int)fb_addr == -1)
  {
    error("unable to map fb data\n");
    return -1;
  }
 // debug("The framebuffer device was mapped to memory successfully.\n");

  return 1;
}

void fb_close(void)
{
  if (fbfd > 0)
    close(fbfd);
}

int fb_get_info(fb_info_t * pi)
{
  if (fbfd < 0) return -1;
  assert(pi);
  pi->addr = fb_addr;
  pi->bpp = vinfo.bits_per_pixel;
  pi->xres = vinfo.xres;
  pi->yres = vinfo.yres;
  return 0;
}

