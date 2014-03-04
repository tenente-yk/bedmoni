#ifndef __DIO_H
#define __DIO_H

#define DIO_FIFO_SIZE  10000

#define DIN_START '{'
#define DIN_END   '}'

void dio_process_data(unsigned char *pp, int len);

#endif // __DIO_H
