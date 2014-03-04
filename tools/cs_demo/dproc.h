#ifndef __DPROC_H
#define __DPROC_H

void dproc_init(void);

void dproc_packet(int id, unsigned long ts, unsigned char * buf, int len);

#endif // __DPROC_H
