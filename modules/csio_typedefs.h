/*! \file csio_typedefs.h
    \brief This file is generated automatically by xml2h
*/

#ifndef  __csio_typedefs_h__
#define  __csio_typedefs_h__

//enums
enum
{
  CSIO_ID_DEBUG = 0,   /*!< 0 */
  CSIO_ID_ECG_C = 1,   /*!< 1 */
  CSIO_ID_SPO2_C = 2,   /*!< 2 */
  CSIO_ID_RESP_C = 3,   /*!< 3 */
  CSIO_ID_CO2_C = 4,   /*!< 4 */
  CSIO_ID_ECG_D = 5,   /*!< 5 */
  CSIO_ID_SPO2_D = 6,   /*!< 6 */
  CSIO_ID_NIBP_D = 7,   /*!< 7 */
  CSIO_ID_RESP_D = 8,   /*!< 8 */
  CSIO_ID_T1T2_D = 9,   /*!< 9 */
  CSIO_ID_CO2_D = 10,   /*!< 10 */
  CSIO_ID_ALARMS = 11,   /*!< 11 */
  CSIO_ID_PAT = 12,   /*!< 12 */
  CSIO_ID_MONICFG = 13,   /*!< 13 */
  CSIO_ID_RPEAK = 14,   /*!< 14 */
  CSIO_ID_ECS = 15,   /*!< 15 */
};

//typedefs

#ifdef WIN32
#pragma pack(1)
#endif

/*! \struct csio_debug_info_t
 * data structure
 */
typedef struct
{
  unsigned char len;
  unsigned char payload[200];
  unsigned char unused[3];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_debug_info_t;

/*! \struct csio_ecgdata_c_t
 * data structure
 */
typedef struct
{
  unsigned char sync;
  unsigned char mode;
  short data[3];
  unsigned char pm;
  unsigned char break_byte;
  unsigned char unused[2];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_ecgdata_c_t;

/*! \struct csio_spo2data_c_t
 * data structure
 */
typedef struct
{
  unsigned char sync;
  unsigned char mode;
  short fpg;
  unsigned char unused[4];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_spo2data_c_t;

/*! \struct csio_respdata_c_t
 * data structure
 */
typedef struct
{
  unsigned char sync;
  unsigned char lead;
  short rpg;
  unsigned char unused[4];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_respdata_c_t;

/*! \struct csio_co2data_c_t
 * data structure
 */
typedef struct
{
  unsigned char sync;
  short data;
  unsigned char unused[5];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_co2data_c_t;

/*! \struct csio_ecgdata_d_t
 * data structure
 */
typedef struct
{
  unsigned short hr;
  unsigned short hr_min;
  unsigned short hr_max;
  short st[7];
  short st_min[7];
  short st_max[7];
  unsigned char hr_src;
  short j_shift;
  unsigned char unused[9];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_ecgdata_d_t;

/*! \struct csio_spo2data_d_t
 * data structure
 */
typedef struct
{
  unsigned short spo2;
  unsigned short spo2_min;
  unsigned short spo2_max;
  unsigned short hr;
  unsigned short hr_min;
  unsigned short hr_max;
  unsigned short stolb;
  unsigned char unused[6];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_spo2data_d_t;

/*! \struct csio_nibpdata_d_t
 * data structure
 */
typedef struct
{
  unsigned short sd;
  unsigned short sd_min;
  unsigned short sd_max;
  unsigned short dd;
  unsigned short dd_min;
  unsigned short dd_max;
  unsigned short md;
  unsigned short md_min;
  unsigned short md_max;
  unsigned short infl;
  unsigned short meas_interval;
  unsigned short hr;
  unsigned long trts;
  unsigned char unused[4];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_nibpdata_d_t;

/*! \struct csio_respdata_d_t
 * data structure
 */
typedef struct
{
  unsigned short br;
  unsigned short br_min;
  unsigned short br_max;
  unsigned char lead;
  unsigned char ap;
  unsigned short ap_max;
  unsigned char unused[2];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_respdata_d_t;

/*! \struct csio_t1t2data_d_t
 * data structure
 */
typedef struct
{
  unsigned short t1;
  unsigned short t1_min;
  unsigned short t1_max;
  unsigned short t2;
  unsigned short t2_min;
  unsigned short t2_max;
  unsigned short dt_max;
  unsigned char unused[2];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_t1t2data_d_t;

/*! \struct csio_co2data_d_t
 * data structure
 */
typedef struct
{
  unsigned short etco2;
  unsigned short etco2_min;
  unsigned short etco2_max;
  unsigned short ico2;
  unsigned short ico2_max;
  unsigned short br;
  unsigned short br_min;
  unsigned short br_max;
  unsigned char unused[8];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_co2data_d_t;

/*! \struct csio_alarms_t
 * data structure
 */
typedef struct
{
  unsigned char set[16];
  unsigned char ena[16];
  unsigned char stat[16];
  unsigned char unused[44];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_alarms_t;

/*! \struct csio_patient_t
 * data structure
 */
typedef struct
{
  unsigned char type;
  unsigned long bedno;
  unsigned long cardno;
  unsigned short w;
  unsigned char h;
  unsigned char unused[4];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_patient_t;

/*! \struct csio_monicfg_t
 * data structure
 */
typedef struct
{
  unsigned long unitmask;
  unsigned long powerstat;
  unsigned char demomask;
  unsigned char unused[3];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_monicfg_t;

/*! \struct csio_rpeak_t
 * data structure
 */
typedef struct
{
  unsigned char id;
  unsigned short hr;
  unsigned char unused[1];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_rpeak_t;

/*! \struct csio_ecs_t
 * data structure
 */
typedef struct
{
  unsigned char id;
  unsigned char payload[5];
  unsigned char unused[2];
}
#ifdef UNIX
__attribute__ ((__packed__))
#endif
csio_ecs_t;

#ifdef WIN32
#pragma pack()
#endif

#endif // __csio_typedefs_h__
