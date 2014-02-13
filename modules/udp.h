#ifndef __UDP_H
#define __UDP_H

//#define UDP_WORKSTATION_COMM_PORT         9930
#define UDP_BEDMONI_CLIENT
//#define UDP_BEDMONI_SERVER

#if defined (UDP_BEDMONI_CLIENT) && defined (UDP_BEDMONI_SERVER)
#error UDP_BEDMONI_CLIENT and UDP_BEDMONI_SERVER are both defined
#endif
#if !defined (UDP_BEDMONI_CLIENT) && !defined (UDP_BEDMONI_SERVER)
#error UDP_BEDMONI_CLIENT and UDP_BEDMONI_SERVER are both undefined
#endif

#if defined (UDP_BEDMONI_SERVER)
int udp_server_init(void);

void udp_server_deinit(void);
#endif

#if defined (UDP_BEDMONI_CLIENT)
int udp_client_init(void);

void udp_client_deinit(void);
#endif

int udp_write(const void * pbuf, int len);

int udp_read(void * pbuf, int len);

int udp_isready(void);

#endif // __UDP_H
