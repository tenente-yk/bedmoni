/*! \file filter.h
 *  \brief Digital filters
 */

#ifndef __FILTER_H
#define __FILTER_H

#include "dview.h"

#define FILTER_MAX_NUM_CHANS       NUM_VIEWS

typedef short data_t;

// ak filter
////////////////////////////////////////////////////////////
/**
 *  \brief data structure for LP-filter for ECG signal, used for pass only QRS complexes on signal (quick QRS filter)
 */
typedef struct
{
  short tbuf[21];
  short coef[21];
} qqrs_filt_data_t;

/**
 *  \brief data structure for 50/60 Hz rejection filter on ECG signal
 */
typedef struct
{
  data_t tbuf1d[4]; // decimator
  data_t tbuf2d[8]; // decimator
  data_t tbuf3[14]; // value-to-value
  data_t tbuf2i[8]; // interpolator
  data_t tbuf1i[4]; // interpolator
  int    coef1[4];
  int    coef2[8];
  int    coef3[14];
  data_t x[4];
  data_t y[4];
  short  pos;
} n5060_filt_data_t;

/**
 *  \brief data structure for LP-filter, fc = 150 Hz
 */
typedef struct
{
  short tbuf[40];
  short coef[40];
} s150_filt_data_t;

/**
 *  \brief data structure for LP-filter, fc = 37 Hz
 */
typedef struct
{
  short tbuf[20];
  short coef[20];
} s37_filt_data_t;

/*! \fn int   qqrs_filt_init(void)
 *  \brief Init quick QRS filter.
 *  \return Returns handle to filter.
 */
int   qqrs_filt_init(void);

/*! \fn void  qqrs_filt_deinit(int fd)
 *  \brief Deinit quick QRS filter.
 *  \param fd Handle to filter.
 */
void  qqrs_filt_deinit(int fd);

/*! \fn void  qqrs_filt_reset(int fd)
 *  \brief Reset quick QRS filter.
 *  \param fd Handle to filter.
 */
void  qqrs_filt_reset(int fd);

/*! \fn short qqrs_filt_stepit(const int fd, short w)
 *  \brief Process filtration for one value via quick QRS filter.
 *  \param fd Handle to filter.
 *  \param w Incoming raw value.
 *  \return Returns filtered value.
 */
short qqrs_filt_stepit(const int fd, short w);

/*! \fn int  n5060_filt_init(void)
 *  \brief Init 50/60 Hz rejectin filter.
 *  \return Returns handle to filter.
 */
int  n5060_filt_init(void);

/*! \fn void n5060_filt_deinit(int fd)
 *  \brief Deinit 50/60 Hz rejectin filter.
 *  \param fd Handle to filter.
 */
void n5060_filt_deinit(int fd);

/*! \fn data_t n5060_filt_stepit(int fd, data_t v)
 *  \brief Process filtration for one value via 50/60 Hz rejectin filter.
 *  \param fd Handle to filter.
 *  \param v Incoming raw value.
 *  \return Returns filtered value.
 */
data_t n5060_filt_stepit(int fd, data_t v);

/*! \fn int   s150_filt_init(void)
 *  \brief Init LP-filter.
 *  \return Returns handle to filter.
 */
int   s150_filt_init(void);

/*! \fn void  s150_filt_deinit(int fd)
 *  \brief Deinit LP-filter.
 *  \param fd Handle to filter.
 */
void  s150_filt_deinit(int fd);

/*! \fn short s150_filt_stepit(int fd, short w)
 *  \brief Process filtration for one value via LP-filter.
 *  \param fd Handle to filter.
 *  \param w Incoming raw value.
 *  \return Returns filtered value.
 */
short s150_filt_stepit(int fd, short w);

/*! \fn int   s37_filt_init(void)
 *  \brief Init LP-filter.
 *  \return Returns handle to filter.
 */
int   s37_filt_init(void);

/*! \fn void  s37_filt_deinit(int fd)
 *  \brief Deinit LP-filter.
 *  \param fd Handle to filter.
 */
void  s37_filt_deinit(int fd);

/*! \fn short s37_filt_stepit(int fd, short w)
 *  \brief Process filtration for one value via LP-filter.
 *  \param fd Handle to filter.
 *  \param w Incoming raw value.
 *  \return Returns filtered value.
 */
short s37_filt_stepit(int fd, short w);

////////////////////////////////////////////////////////////

/**
 *  \brief data structure for HP-filter
 */
typedef struct
{
  float A01;
  float A11;
  float D01;
  float D11;
  float C11;
  float x[2];
  float y[2];
} hp_filt_data_t;

/*! \fn int hp_filt_init(double freq)
 *  \brief Init HP-filter.
 *  \param freq Cut freqeuncy.
 *  \return Returns handle to filter.
 */
int hp_filt_init(double freq);

/*! \fn void hp_filt_deinit(int fd)
 *  \brief Deinit HP-filter.
 *  \param fd Handle to filter.
 */
void hp_filt_deinit(int fd);

/*! \fn int hp_filt_stepit(int chno, int v)
 *  \brief Process filtration for one value via HP-filter.
 *  \param fd Handle to filter.
 *  \param v Incoming raw value.
 *  \return Returns filtered value.
 */
int hp_filt_stepit(int fd, int v);

/**
 *  \brief data structure for LP-filter (for impedance signal)
 */
typedef struct
{
  int   ptr;
  short tbuf[40];
} resp_filt_data_t;

/*! \fn int resp_filt_init(double freq)
 *  \brief Init LP-filter for Resp.
 *  \param freq Cut freqeuncy.
 *  \return Returns handle to filter.
 */
int resp_filt_init(double freq);

/*! \fn void resp_filt_deinit(int fd)
 *  \brief Deinit LP-filter for Resp.
 *  \param fd Handle to filter.
 */
void resp_filt_deinit(int fd);

/*! \fn int resp_filt_stepit(int fd, int v)
 *  \brief Process filtration for one impedance value via LP-filter.
 *  \param fd Handle to filter.
 *  \param v Incoming raw value.
 *  \return Returns filtered value.
 */
int resp_filt_stepit(int fd, int v);

#endif // __FILTER_H
