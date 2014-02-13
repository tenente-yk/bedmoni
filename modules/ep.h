#ifndef __EP_H
#define __EP_H

//#define ST_UNDEF_VALUE      (100*UNDEF_VALUE)

void ep_process_packet(unsigned char *data, int len);

int  ep_is_ok(void);

void ecg_on_no_ep(void);

#endif // __EP_H
