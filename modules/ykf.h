/*! \file ykf.h
 *  \brief Functions to operate with YKF fonts
 */

#ifndef __YKF_H
#define __YKF_H

/*
 ykf file structure
 OFFSET             DESC
 0-3                "YKF\0"
 4                  font height (unsigned char)
 5                  font width (unsigned char)
 6                  number of symbols N in a font (unsigned char)
 7                  reserved (unsigned char) - 0x00
 8                  address of beginning first symbol data (unsigned long) - addr1
 12                 address of beginning second symbol data (unsigned long) - addr2
 .
 .
 .
 8+4*N              address of beginning N symbol data - addrN
 addr1              first symbol data
 .
 .
 .

symbol data structure
 OFFSET             DESC
 0                  height
 1                  width
 2                  nblocks
 3                  size (unsigned short)
 4                  data (size bytes of symbol data)
 .
 .

*/

/**
 *  \brief Setting this define forces to load font data in RAM
 */
#define USE_RAMFONT

/**
 *  \brief YKF font info structure
 */
typedef struct
{
  unsigned char font_h;
  unsigned char font_w;
  unsigned char nsymbols;
  unsigned char unused[1];
  FILE *        f;
  int           fd;
} ykf_info_t;

/**
 *  \brief YKF font char info structure
 */
typedef struct
{
  unsigned char  h;
  unsigned char  w;
  unsigned char  nblocks;
  unsigned short size;
  unsigned char  unused[3];
  unsigned char  data[2000];
} ykf_charinfo_t;

/*! \fn int  ykf_open(char * fname)
 *  \brief Open YKF font by file name.
 *  \param fname Font filename.
 *  \return Returns pointer to \a ykf_info_t on success (handle), 0 - otherwise.
 */
int  ykf_open(char * fname);

/*! \fn void ykf_close(int fd)
 *  \brief Close YKF font by handle.
 *  \param fd Handle to opened font.
 */
void ykf_close(int fd);

/*! \fn int  ykf_get_font_h(int fd)
 *  \brief Get font height.
 *  \param fd Handle to opened font.
 *  \return Returns font height.
 */
int  ykf_get_font_h(int fd);

/*! \fn int  ykf_get_font_w(int fd)
 *  \brief Get font width.
 *  \param fd Handle to opened font.
 *  \return Returns font width.
 */
int  ykf_get_font_w(int fd);

/*! \fn int  ykf_get_charinfo(int fd, int c, ykf_charinfo_t * pci)
 *  \brief Get font char information.
 *  \param fd Handle to opened font.
 *  \param c Char to get information.
 *  \param pci Pointer to \a ykf_charinfo_t.
 *  \return Returns number of read bytes.
 */
int  ykf_get_charinfo(int fd, int c, ykf_charinfo_t * pci);

#endif // __YKF_H
