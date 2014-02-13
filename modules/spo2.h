#ifndef __SPO2_H
#define __SPO2_H

#define SPO2_USART USART0

// Definitions for  Send Info POX protokol
// Content of POX Status byte: 
#define SIPOX_NoSensor          0x80 // ����� ������ �� ��������� � ������.
#define SIPOX_NoPatient         0x40 // ������ ��������� � ������, �� ����� � ������� �����������
#define SIPOX_TuneProcess       0x20 // ����� ���� �������������� ����� � �������� ������
#define SIPOX_CannotTune        0x10 // �� ���� ����������� ��������� ��������� �������
#define SIPOX_DarkObject        0x08 // ������ ����������� ��� ����� � �������
#define SIPOX_TransparentObject 0x04 // ������ ������� ��������� ��������� ��������� �������
#define SIPOX_BadSignal         0x02 // ���������� ��������� ��������� ��������� ��������� �������
#define SIPOX_PulseBip          0x01 // ��������� ����� ���� ������ (�����).
// Send Info Package (send each 10 ms)
#define SIPOX_OFFSET_PILA       0    // (0- 255)
#define SIPOX_OFFSET_STAT       1    // POX Status byte (see above)
#define SIPOX_OFFSET_SPO2       2    // Saturation Value (0 to 100) 101- undefined value 
#define SIPOX_OFFSET_PR_LOW     3    // Pulse Rate low byte (0 to 300) 0- undefined value
#define SIPOX_OFFSET_PR_HIGH    4    // Pulse Rate high byte
#define SIPOX_OFFSET_PERF       5    // Stolbik (Perfussion) ������ ���������� ������ (0 to 254) 0- ����� �� �������������� � ������� 8 ������ ��� �����
#define SIPOX_OFFSET_PLET       6    // ������������������  (0 - 255) 
#define SIPOX_OFFSET_RSRV       7    // Reserved. Allways = 0xe7
#define SIPOX_OFFSET_CRC        8    // simple summ

#define SPO2_PACKET_SIZE        (9)
#define SPO2_DATABUF_MAXSIZE    (5*SPO2_PACKET_SIZE)

#if defined (LPC2468) || defined (LPC2478)
#define SPO2_TIMEOUT            2000 // in ms
#define SPO2_SLEEP_DELAY        (4*configTICK_RATE_HZ/1000)
#endif

// spo2 commands range 0xf0 - 0xff
#define SPO2_SET_SCALE          0xF0

#define SPO2_AUTOSCALE          0x00
#define SPO2_NOAUTOSCALE        0x01
#define SPO2_PILASCALE          0x02

typedef struct
{ // MLB
  unsigned char pulse_bip:   1;
  unsigned char bad_signal:  1;
  unsigned char transp_obj:  1;
  unsigned char dark_obj:    1;
  unsigned char tune_err:    1;
  unsigned char tune_proc:   1;
  unsigned char no_patient:  1;
  unsigned char no_sensor:   1;
} // MSB
spo2_stat_bits_t;

typedef struct
{ // MLB
  unsigned short hr:         14;
  unsigned short mode:       2;
  // MSB
} hr_n_mode_t;

#pragma pack(1)
typedef struct
{
  unsigned char       sync;
  union
  {
    spo2_stat_bits_t  stat_bits;
    unsigned char     stat;
  };
  unsigned char       sat;
  unsigned char       stolb;
  hr_n_mode_t         hr_mode;
  short               fpg;
  unsigned char       crc;
} spo2_packet_t;
#pragma pack()

#if defined (LPC2468) || defined (LPC2478)
void spo2_task(void * pvParameters);
#endif

#if defined (UNIX) || defined (WIN32)
void spo2_process_packet(unsigned char *data, int len);
#endif

#endif // __SPO2_H
