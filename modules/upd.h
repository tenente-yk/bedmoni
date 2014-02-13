#ifndef __UPD_H
#define __UPD_H

#define UPD_MSG_START    "start"

#define UPD_STATE_IDLE            0
#define UPD_STATE_INPROGRESS      1
#define UPD_STATE_FINISHED        2
#define UPD_STATE_WAITFORCONN     3

#define UPD_BYTE_START            '['
#define UPD_BYTE_END              ']'

#define UPD_CODE_OK               'U'
#define UPD_CODE_FAIL             'N'
#define UPD_CODE_BUSY             'B'
#define UPD_CODE_END              'E'

int  upd_client_open(char * ips, int portno);

void upd_client_close(int sockfd);

void upd_stepit(int sockfd);

int  upd_server(int portno);

int  upd_get_state(void);

int  upd_get_progress(void);

void upd_set_progress(int pos);

#endif // __UPD_H
