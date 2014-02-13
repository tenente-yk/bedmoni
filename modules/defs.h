#ifndef __DEFS_H
#define __DEFS_H

/*! \file defs.h
    \brief WIN32 common definitions
*/

#ifndef WIN32

/*!
  \def RGB(r,g,b)
  Converts from red \a r, green \a g and blue \a b color bytes to unsigned long value .
*/
/*!
  \def BGR(b,g,r)
  Converts from blue \a b, green \a g and red \a r color bytes to unsigned long value .
*/
#ifndef RGB
#define BGR(b,g,r) ((unsigned long)(((unsigned char)(r)|\
                                (((unsigned long)(unsigned char)(g))<<8))|\
                                (((unsigned long)(unsigned char)(b))<<16)|\
                                (((unsigned long)(unsigned char)(0xff))<<24)))

#if 1
#define RGB(r,g,b) ((unsigned long)(((unsigned char)(r)|\
                                (((unsigned long)(unsigned char)(g))<<8))|\
                                (((unsigned long)(unsigned char)(b))<<16)|\
                                (((unsigned long)(unsigned char)(0xff))<<24)))
#else
#define RGB(r,g,b)    BGR(r,g,b)
#endif

#endif // RGB


/*!
  \def MAKELONG(a,b)
  macro creates a LONG value by concatenating the specified values \a a ( low word) and \a b (high word)
*/
#ifndef MAKELONG
//#define MAKELONG(a, b) (a | (b << 16))//((long)(((unsigned short)(a)) | ((unsigned long)((unsigned short))) << 16 ))
#define MAKELONG(a, b)      ((long)(((unsigned short)(a)) | ((unsigned long)((unsigned short)(b))) << 16))
#endif

/*!
  \def MAKEWORD(a,b)
  macro creates a WORD value by concatenating the specified values \a a ( low byte) and \a b (high byte)
*/
#ifndef MAKEWORD
#define MAKEWORD(a,b)       ((short)(((unsigned char)(a)) | ((unsigned short)((unsigned char)(b))) << 8))
#endif

/*!
  \def LOWORD(l)
  macro retrieves the low-order word from the specified value
*/
#ifndef LOWORD
#define LOWORD(l) ((unsigned short)(l))
#endif

/*!
  \def HIWORD(l)
  macro retrieves the high-order word from the specified value
*/
#ifndef HIWORD
#define HIWORD(l) (unsigned short)(((unsigned long)l>>16) & 0xffff) //((unsigned short)(((unsigned long)(l) >> 16) & 0xffff))
#endif

#define DT_LEFT              0x00000000
#define DT_CENTER            0x00000001
#define DT_RIGHT             0x00000002
#define DT_CALCRECT          0x00000400


typedef unsigned long        COLORREF;
typedef int                  BOOL;
typedef unsigned int         UINT;

#define TRUE  (1)
#define FALSE (0)

#define PBS_VERTICAL 0

#define SW_SHOW      1
#define SW_HIDE      0

#define SWP_NOMOVE   1
#define SWP_NOSIZE   2

#define CALLBACK

#define min(a,b) (a)<(b) ? a : b
#define max(a,b) (a)>(b) ? a : b

#else
//#include <windows.h>
#define __FUNCTION__       "__FUNCTION__"
#define snprintf           _snprintf
#define vsnprintf          _vsnprintf
#define mkdir(path, perm)  _mkdir(path)

typedef unsigned long        COLORREF;
/*!
  \def BGR(b,g,r)
  Converts from blue \a b, green \a g and red \a r color bytes to unsigned long value .
*/
#ifndef RGB
#define BGR(b,g,r) ((unsigned long)(((unsigned char)(r)|\
                                (((unsigned long)(unsigned char)(g))<<8))|\
                                (((unsigned long)(unsigned char)(b))<<16)|\
                                (((unsigned long)(unsigned char)(0xff))<<24)))

#if 1
#define RGB(r,g,b) ((unsigned long)(((unsigned char)(r)|\
                                (((unsigned long)(unsigned char)(g))<<8))|\
                                (((unsigned long)(unsigned char)(b))<<16)|\
                                (((unsigned long)(unsigned char)(0xff))<<24)))
#else
#define RGB(r,g,b)    BGR(r,g,b)
#endif

#endif // RGB


#ifndef MAKELONG
//#define MAKELONG(a, b) (a | (b << 16))//((long)(((unsigned short)(a)) | ((unsigned long)((unsigned short))) << 16 ))
#define MAKELONG(a, b)      ((long)(((unsigned short)(a)) | ((unsigned long)((unsigned short)(b))) << 16))
#endif

#ifndef MAKEWORD
#define MAKEWORD(a,b)       ((short)(((unsigned char)(a)) | ((unsigned short)((unsigned char)(b))) << 8))
#endif

#ifndef LOWORD
#define LOWORD(l) ((unsigned short)(l))
#endif

#ifndef HIWORD
#define HIWORD(l) (unsigned short)(((unsigned long)l>>16) & 0xffff) //((unsigned short)(((unsigned long)(l) >> 16) & 0xffff))
#endif

#define DT_LEFT              0x00000000
#define DT_CENTER            0x00000001
#define DT_CALCRECT          0x00000400


typedef unsigned long        COLORREF;
typedef int                  BOOL;
typedef unsigned int         UINT;

#define PBS_VERTICAL 0

/*!
  \def min(a,b)
  Computes the minimum of \a a and \a b.
*/
#ifndef min
#define min(a,b) (a)<(b) ? a : b
#endif
/*!
  \def max(a,b)
  Computes the maximum of \a a and \a b.
*/
#ifndef max
#define max(a,b) (a)>(b) ? a : b
#endif

#ifndef SW_SHOW
#define SW_SHOW              1
#endif
#ifndef SW_HIDE
#define SW_HIDE              0
#endif

//typedef long                 LRESULT;
//typedef unsigned int         HWND;

#endif // ! defined (WIN32)

#define DT_VERTICAL          0x80000000

#endif // __DEFS_H

