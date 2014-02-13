/*! \file port.h
    \brief Serial port functions.
*/

#ifndef __PORT_H
#define __PORT_H

#ifdef UNIX
#include <termios.h>
#endif

/*!
  \def RATES
  Max number of supported baudrates.
*/
#define RATES 13

/*! \enum br_t
 * Standard baudrates for serial port communication.
 */
typedef enum 
{
  BR300,      /*!< 0 */  ///< - 300 bps
  BR600,      /*!< 1 */  ///< - 600 bps
  BR1200,    /*!< 2 */  ///< - 1200 bps
  BR2400,    /*!< 3 */  ///< - 2400 bps
  BR4800,    /*!< 4 */  ///< - 4800 bps
  BR9600,    /*!< 5 */  ///< - 9600 bps
  BR19200,   /*!< 6 */  ///< - 19200 bps
  BR38400,   /*!< 7 */  ///< - 38400 bps
  BR57600,   /*!< 8 */  ///< - 57600 bps
  BR115200,  /*!< 9 */  ///< - 115200 bps
  BR230400,  /*!< 10 */  ///< - 230400 bps
  BR460800,  /*!< 11 */  ///< - 460800 bps
  BR921600,  /*!< 12 */  ///< - 921600 bps
} br_t;

/*! \struct dl_t
 * H/W datalines' state.
 */
#pragma pack(1)
typedef struct
{
  unsigned char setdtr : 1; ///<- Sends the DTR (data-terminal-ready) signal.
  unsigned char clrdtr : 1; ///<- Clears the DTR (data-terminal-ready) signal.
  unsigned char setrts : 1; ///<- Sends the RTS (request-to-send) signal.
  unsigned char clrrts : 1; ///<- Clears the RTS (request-to-send) signal.
  unsigned char setxon : 1; ///<- Causes transmission to act as if an XON character has been received.
  unsigned char setxoff : 1; ///<- Causes transmission to act as if an XOFF character has been received.
  unsigned char setbreak : 1; ///<- Suspends character transmission and places the transmission line in a break state.
  unsigned char clrbreak : 1; ///<- Restores character transmission and places the transmission line in a nonbreak state.
} dl_t;
#pragma pack()

/****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
/****************************************************************************/

/*! \fn int com_open( char *port_name, br_t port_rate, char *settings)
 *  \brief Open serial port.
 *  \param port_name name of a port to openning (e.g. /dev/ttyS0).
 *  \param port_rate baudrate.
 *  \param settings not used, must be 0.
 *  \return port descriptor: other than 0 if success, 0 of less - if failure.
 */
int com_open( char *port_name, br_t port_rate, char *settings);

/*! \fn int com_close(int fd)
 *  \brief Close serial port.
 *  \param fd descriptor of the opened port
 *  \return other than 0 if success, 0 - if failure.
 */
int com_close(int);

/*! \fn int com_waitfor_ch(int fd, char *s, char inp_c)
 *  \brief Wait for character during 1 second from opened port.
 *  \param fd descriptor of the opened port
 *  \param s buffer for input character
 *  \param inp_c sends this character if no any character received
 *  \return other than 0 if success, 0 - if failure.
 */
int com_waitfor_ch(int fd, char *s, char inp_c);

/*! \fn int com_write(int fd, const char *arr, int size)
 *  \brief Write characters to serial port
 *  \param fd descriptor of the opened port
 *  \param arr output buffer
 *  \param size number of bytes to send
 *  \return number successfully sent bytes if success, -1 if failure.
 */
int com_write(int fd, const char *arr, int size);

/*! \fn int com_read(int fd, char *arr, int size)
 *  \brief Read characters from serial port
 *  \param fd descriptor of the opened port
 *  \param arr input buffer
 *  \param size number of bytes to read
 *  \return number successfully read bytes if success, -1 if failure.
 */
int com_read(int fd, char *arr, int size);

/*! \fn int com_write_ch(int fd, char ch)
 *  \brief Write character \a ch to serial port
 *  \param fd descriptor of the opened port
 *  \param ch output character
 *  \return number successfully sent bytes if success, -1 if failure.
 */
int com_write_ch(int fd, char ch);

/*! \fn int com_recv(int fd, char *buf, int len)
 *  \brief Read characters from serial port
 *  \param fd descriptor of the opened port
 *  \param buf input buffer
 *  \param len number of bytes to read
 *  \return number successfully read bytes if success, -1 if failure.
 */
int com_recv(int fd, char *buf, int len);

/*! \fn int com_setbaud(int fd, br_t port_rate)
 *  \brief Set communication baudrate
 *  \param fd descriptor of the opened port
 *  \param port_rate baudrate to set
 *  \return 1 if success, 0 if failure.
 */
int com_setbaud(int fd, br_t port_rate);

/*! \fn int com_purge(int fd)
 *  \brief Purge all data in serial port buffers
 *  \param fd descriptor of the opened port
 *  \return 0 if success, -1if failure.
 */
int com_purge(int fd);

/*! \fn int com_flush(int fd)
 *  \brief Flush all data to serial port
 *  \param fd descriptor of the opened port
 *  \return 0 if success, -1 if failure.
 */
int com_flush(int fd);

/*! \fn com_setdataline(int fd, dl_t dl)
 *  \brief Set hw data lines to corresponding state
 *  \param fd descriptor of the opened port
 *  \param dl data line state
 *  \return 0 if success, -1 if failure.
 */
int com_setdataline(int fd, dl_t dl);

#ifdef UNIX
/*! \fn int com_open_2(char *port_name, unsigned long tio_rate)
 *  \brief Open serial port.
 *  \param port_name name of a port to openning (e.g. /dev/ttyS0).
 *  \param tio_rate custom baudrate.
 *  \return port descriptor: other than 0 if success, 0 of less - if failure.
 */
int com_open_2(char *port_name, unsigned long tio_rate);
#endif
#ifdef WIN32
/*! \fn int com_enum(int *pnums, int size)
 *  \brief Enumerate serial ports.
 *  \param pnums storage for existing serial port numbers.
 *  \param size max number of items in pnums array.
 *  \return 1 if success, 0 if failure.
 */
int com_enum(int *pnums, int size);
#endif


/****************************************************************************/
#ifdef __cplusplus
}
#endif
/****************************************************************************/

#endif // __PORT_H
/****************************************************************************/
/****************************************************************************/
