/*! \file grph.h
 *  \brief Graphic primitives
 */

#ifndef __LCD_GRPH_H
#define __LCD_GRPH_H

#include "defs.h"

#if !defined (ARM)
#define FEATURE_BPP32
#endif

#define GRPH_MAX_LINE_LEN             1000

#ifndef RGB2PIXEL565
#define RGB2PIXEL565(r,g,b)  \
 ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))
#endif

#ifndef COLOR2PIXEL565
#define COLOR2PIXEL565(c)    \
 ((((c) & 0xf8) << 8) | (((c) & 0xfc00) >> 5) | (((c) & 0xf80000) >> 19))
#endif

#ifndef COLOR2PIXEL8888
#define COLOR2PIXEL8888(c)    \
 ((((c) & 0xff) << 16) | ((c) & 0x0000ff00ul) | (((c) & 0xff0000) >> 16))
#endif

#ifndef PIXEL565TOCOLORVAL
#define PIXEL565TOCOLORVAL(p)    \
 (0x00000000ul | (((p) & 0xf800u) << 8) | (((p) & 0x07e0u) << 5) | (((p) & 0x1ful) << 3))
#endif

/**
 *  \brief Font list
 */
typedef enum
{
  ARIAL_8,
  ARIAL_8B,
  ARIAL_9,
  ARIAL_10,
  ARIAL_12,
  ARIAL_14,
  ARIAL_18,
  ARIAL_28,
  ARIAL_36,
  ARIAL_48,
  ARIAL_60,
  MONOS_11,
  MONOS_10,
  ARIAL_N_12,
  ARIAL_N_24,
  ARIAL_N_30,
  ARIAL_N_49,
  ARIAL_N_70,
  ARIAL_N_84,
  ARIAL_N_114,
  NUM_FONTS,
} font_id;

enum
{
  GRPH_SRCCOPY,
  GRPH_SRCPAINT,
};

enum
{
  PS_SOLID = 0,
  PS_DOT,
  PS_NUM_STYLES,
};

/**
 *  \brief Font data structure
 */
typedef struct
{
  char          name[30];
  unsigned char id;
  char          fname[30];
  unsigned char dx; // ���������� ����� ��������� � ��������
} gui_font_t;

/**
 *  \brief Rectangular data structure
 */
typedef struct
{
  int x0;
  int y0;
  int x1;
  int y1;
} rect_t;

/**
 *  \brief Framebuffer data structure
 */
typedef struct
{
  int    xres;
  int    yres;
  int    bpp;
  void * addr;
} grph_screeninfo_t;

typedef grph_screeninfo_t * PGSI;

/**
 *  \brief Graphic context data structure
 */
typedef struct
{
  PGSI          pgsi;   // graph screen info
  short         x;
  short         y;
  short         cx;
  short         cy;
  int           xres;
  int           yres;
  void *        addr;
  unsigned long fgcolor;
  unsigned long bkcolor;
  int           bpp;    // bits per pixel
  unsigned int  font_id;
  int           ps;     // pen style
} gdc_t;

typedef gdc_t * PGDC;

/*! \fn int  grph_init(void)
 *  \brief Init graphic primitives library.
 *  \return Returns zero.
 */
int  grph_init(void);

/*! \fn void grph_deinit(void)
 *  \brief Deinit graphic primitives library.
 */
void grph_deinit(void);

/*! \fn int  grph_create(PGSI pgsi, int flags)
 *  \brief Creates graphic context based on framebuffer device.
 *  \param pgsi Pointer to \a grph_screeninfo_t.
 *  \param flags Settings field.
 *  \return Returns handle to created graphic context.
 */
int  grph_create(PGSI pgsi, int flags);

/*! \fn void grph_destroy(int fd)
 *  \brief Destroys created graphic context based on framebuffer device.
 *  \param fd Graphic context handle.
 */
void grph_destroy(int fd);

/*! \fn void grph_line(int fd, int x0, int y0, int x1, int y1)
 *  \brief Draws line on graphic context.
 *  \param fd Graphic context handle.
 *  \param x0 Specifies the x-coordinate, in logical units, of the line's starting point.
 *  \param y0 Specifies the y-coordinate, in logical units, of the line's starting point.
 *  \param x1 Specifies the x-coordinate, in logical units, of the line's ending point.
 *  \param y1 Specifies the y-coordinate, in logical units, of the line's ending point.
 */
void grph_line(int fd, int x0, int y0, int x1, int y1);

/*! \fn void grph_line_bound(int fd, int x0, int y0, int x1, int y1, int yl, int yu)
 *  \brief Draws line on graphic context.
 *  \param fd Graphic context handle.
 *  \param x0 Specifies the x-coordinate, in logical units, of the line's starting point.
 *  \param y0 Specifies the y-coordinate, in logical units, of the line's starting point.
 *  \param x1 Specifies the x-coordinate, in logical units, of the line's ending point.
 *  \param y1 Specifies the y-coordinate, in logical units, of the line's ending point.
 *  \param yl Specifies the y-coordinate, in logical units, of the lower boundary.
 *  \param yu Specifies the y-coordinate, in logical units, of the upper boundary.
 */
void grph_line_bound(int fd, int x0, int y0, int x1, int y1, int yl, int yu);

/*! \fn void grph_filldata16(int fd, int x, int y, int cx, int cy, void * data)
 *  \brief Fills mentioned rectangular with precreated data for 16 bits per pixel graphic context.
 *  \param fd Graphic context handle.
 *  \param x Specifies the x-coordinate, in logical units, of the left upper point.
 *  \param y Specifies the y-coordinate, in logical units, of the left upper point.
 *  \param cx Specifies width, in logical units.
 *  \param cy Specifies height, in logical units.
 *  \param data Data to be filled in mentioned rectangular.
 */
void grph_filldata16(int fd, int x, int y, int cx, int cy, void * data);

/*! \fn void grph_filldatastretch16(int fd, int x, int y, int cx_src, int cy_src, int cx_dst, int cy_dst, void * data)
 *  \brief Stretches mentioned rectangular with precreated data for 16 bits per pixel graphic context.
 *  \param fd Graphic context handle.
 *  \param x Specifies the x-coordinate, in logical units, of the left upper point.
 *  \param y Specifies the y-coordinate, in logical units, of the left upper point.
 *  \param cx_src Specifies source width, in logical units.
 *  \param cy_src Specifies source height, in logical units.
 *  \param cx_dst Specifies destination width, in logical units.
 *  \param cy_dst Specifies destination height, in logical units.
 *  \param data Data to be filled in mentioned rectangular.
 */
void grph_filldatastretch16(int fd, int x, int y, int cx_src, int cy_src, int cx_dst, int cy_dst, void * data);

/*! \fn void grph_fillrect(int fd, int x, int y, int cx, int cy, unsigned int RGB)
 *  \brief Fills mentioned rectangular with determined color.
 *  \param fd Graphic context handle.
 *  \param x Specifies the x-coordinate, in logical units, of the upper-left corner.
 *  \param y Specifies the y-coordinate, in logical units, of the upper-left corner.
 *  \param cx Specifies width, in logical units.
 *  \param cy Specifies height, in logical units.
 *  \param RGB Color RGB (8/8/8).
 */
void grph_fillrect(int fd, int x, int y, int cx, int cy, unsigned int RGB);

/*! \fn void grph_fillroundedrect(int fd, int x, int y, int cx, int cy, unsigned int RGB)
 *  \brief Fills mentioned rounded rectangular with determined color.
 *  \param fd Graphic context handle.
 *  \param x Specifies the x-coordinate, in logical units, of the upper-left corner.
 *  \param y Specifies the y-coordinate, in logical units, of the upper-left corner.
 *  \param cx Specifies width, in logical units.
 *  \param cy Specifies height, in logical units.
 *  \param RGB Color RGB (8/8/8).
 */
void grph_fillroundedrect(int fd, int x, int y, int cx, int cy, unsigned int RGB);


/*! \fn void grph_textout(int fd, int x, int y, char * s, int len)
 *  \brief Writes a character string on graphic context.
 *  \param fd Graphic context handle.
 *  \param x Specifies the x-coordinate of starting position.
 *  \param y Specifies the y-coordinate of starting position.
 *  \param s Character string.
 *  \param len Number of characters.
 */
void grph_textout(int fd, int x, int y, char * s, int len);

/*! \fn void grph_drawtext(int fd, char * s, int count, rect_t *lrc, unsigned int format)
 *  \brief Draws formatted text on graphic context.
 *  \param fd Graphic context handle.
 *  \param s Text to draw.
 *  \param count Text length.
 *  \param lrc Formatting dimensions.
 *  \param format Text drawing options.
 */
void grph_drawtext(int fd, char * s, int count, rect_t *lrc, unsigned int format);

/*! \fn void grph_circle(int fd, short x0, short y0, short r)
 *  \brief Draws circle.
 *  \param fd Graphic context handle.
 *  \param x0 Specifies the x-coordinate of starting position.
 *  \param y0 Specifies the y-coordinate of starting position.
 *  \param r Specifies the radius of circle.
 */
void grph_circle(int fd, short x0, short y0, short r);

/*! \fn void grph_setfgcolor(int fd, unsigned long rgb)
 *  \brief Set current foreground color.
 *  \param fd Graphic context handle.
 *  \param rgb Foreground color value.
 */
void grph_setfgcolor(int fd, unsigned long rgb);

/*! \fn void grph_setbkcolor(int fd, unsigned long rgb)
 *  \brief Set current background color.
 *  \param fd Graphic context handle.
 *  \param rgb Background color value.
 */
void grph_setbkcolor(int fd, unsigned long rgb);

/*! \fn void grph_setfont(int fd, unsigned int font_id)
 *  \brief Set current font.
 *  \param fd Graphic context handle.
 *  \param font_id Font identificator.
 */
void grph_setfont(int fd, unsigned int font_id);

/*! \fn int  grph_setfont_sh(int fd, unsigned int font_id, int h)
 *  \brief Set current font based on font's height.
 *  \param fd Graphic context handle.
 *  \param name Font name.
 *  \param h Proposed font height.
 */
int  grph_setfont_sh(int fd, char* name, int h);

/*! \fn void grph_select_pen(int fd, int ps)
 *  \brief Set pen style for mentioned graphic context.
 *  \param fd Graphic context handle.
 *  \param ps Pen style.
 */
void grph_select_pen(int fd, int ps);

/*! \fn void grph_bitblt(int fd_to, int x, int y, int cx, int cy, int fd_from, int x1, int y1, int rop)
 *  \brief Copies a data from the source graphic context to this current graphic context.
 *  \param fd_to Handle to destination graphic context.
 *  \param x X-coord of destination upper-left corner.
 *  \param y Y-coord of destination upper-left corner.
 *  \param cx Hidth of destination rectangle.
 *  \param cy Height of destination rectangle.
 *  \param fd_from Handle to source graphic context.
 *  \param x1 X-coord of source upper-left corner.
 *  \param y1 Y-coord of source upper-left corner.
 *  \param rop Raster operation mode.
 */
void grph_bitblt(int fd_to, int x, int y, int cx, int cy, int fd_from, int x1, int y1, int rop);

/*! \fn int  grph_createdc(int fd, int x, int y, int cx, int cy, int flags)
 *  \brief Creates child graphic context with mentioned dimensions.
 *  \param fd Handle to parent graphic context.
 *  \param x X-coord of upper-left corner.
 *  \param y Y-coord of upper-left corner.
 *  \param cx Width of rectangle.
 *  \param cy Height of rectangle.
 *  \return Returns graphic context handle.
 */
int  grph_createdc(int fd, int x, int y, int cx, int cy, int flags);

/*! \fn void grph_releasedc(int fd)
 *  \brief Releases graphic context.
 *  \param fd Handle to graphic context.
 */
void grph_releasedc(int fd);

/*! \fn void grph_changedc(int fd, PGSI pgsi)
 *  \brief Locates graphic context from device.
 *  \param fd Handle to graphic context.
 *  \param pgsi Pointer to \a grph_screeninfo_t.
 */
void grph_changedc(int fd, PGSI pgsi);

/*! \fn void grph_filltriangle(int fd, int x1, int y1, int x2, int y2, int x3, int y3)
 *  \brief Draws filled triangle.
 *  \param fd Handle to graphic context.
 *  \param x1 X-coord of point 1.
 *  \param y1 Y-coord of point 1.
 *  \param x2 X-coord of point 2.
 *  \param y2 Y-coord of point 2.
 *  \param x3 X-coord of point 3.
 *  \param y3 Y-coord of point 3.
 */
void grph_filltriangle(int fd, int x1, int y1, int x2, int y2, int x3, int y3);

#endif // __LCD_GRPH_H
