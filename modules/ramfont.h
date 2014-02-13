#ifndef __RAMFONT_H
#define __RAMFONT_H

#include <stdio.h>
#include "ykf.h"

int  ramfont_create(FILE * f);

void ramfont_destroy(int fd);

int  ramfont_get_font_h(int fd);

int  ramfont_get_font_w(int fd);

int  ramfont_get_charinfo(int fd, int c, ykf_charinfo_t * pci);

#endif // __RAMFONT_H
