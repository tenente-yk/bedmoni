#include <stdio.h>
#include <string.h>

#include "cframe.h"
#include "uframe.h"
#include "mframe.h"
#include "mainmenu.h"
#include "grph.h"

static void main_none_func(void*);
static void main_ecg_1_func(void*);
static void main_ecg_2_func(void*);
static void main_ecg_3_func(void*);
static void main_ecg_4_func(void*);
static void main_ecg_5_func(void*);
static void main_ecg_6_func(void*);
static void main_ecg_7_func(void*);
static void main_ecg_func(void*);
static void main_ecg_amp_func(void*);
static void main_ecg_spd_func(void*);
static void main_spo2_func(void*);
static void main_spo2_c_func(void*);
static void main_spo2_amp_func(void*);
static void main_spo2_spd_func(void*);
static void main_resp_c_func(void*);
static void main_resp_amp_func(void*);
static void main_resp_spd_func(void*);
static void main_nibp_func(void*);
static void main_resp_func(void*);
static void main_t1t2_func(void*);
static void main_co2_func(void*);
static void main_co2_c_func(void*);
static void main_co2_amp_func(void*);
static void main_co2_spd_func(void*);
static void main_mode_func(void*);

inpfoc_funclist_t inpfoc_main_funclist[MAIN_NUM_ITEMS+1] = 
{
  { MAIN_NONE,      main_none_func       },
  { MAIN_MODE,      main_mode_func       },
  { MAIN_ECG_1,     main_ecg_1_func      },
  { MAIN_ECG_2,     main_ecg_2_func      },
  { MAIN_ECG_3,     main_ecg_3_func      },
  { MAIN_ECG_4,     main_ecg_4_func      },
  { MAIN_ECG_5,     main_ecg_5_func      },
  { MAIN_ECG_6,     main_ecg_6_func      },
  { MAIN_ECG_7,     main_ecg_7_func      },
  { MAIN_ECG_AMP,   main_ecg_amp_func    },
  { MAIN_ECG_SPD,   main_ecg_spd_func    },
  { MAIN_ECG_NEO,   main_none_func       },
  { MAIN_SPO2_C,    main_spo2_c_func     },
  { MAIN_SPO2_AMP,  main_spo2_amp_func   },
  { MAIN_SPO2_SPD,  main_spo2_spd_func   },
  { MAIN_RESP_C,    main_resp_c_func     },
  { MAIN_RESP_AMP,  main_resp_amp_func   },
  { MAIN_RESP_SPD,  main_resp_spd_func   },
  { MAIN_RESP_C,    main_resp_func       },
  { MAIN_ECG,       main_ecg_func        },
  { MAIN_NIBP,      main_nibp_func       },
  { MAIN_RESP,      main_resp_func       },
  { MAIN_T1T2,      main_t1t2_func       },
  { MAIN_SPO2,      main_spo2_func       },
  { MAIN_CO2,       main_co2_func        },
  { MAIN_CO2_C,     main_co2_c_func      },
  { MAIN_CO2_AMP,   main_co2_amp_func    },
  { MAIN_CO2_SPD,   main_co2_spd_func    },
  { -1            , main_none_func       }, // -1 must be last
};

static void main_none_func(void * parg)
{

}

static void main_ecg_1_func(void * parg)
{
  cframe_command(CFRAME_ECGLEAD, (void*)MAKELONG(MAIN_ECG_1 - MAIN_ECG_1, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_ecg_2_func(void * parg)
{
  cframe_command(CFRAME_ECGLEAD, (void*)MAKELONG(MAIN_ECG_2 - MAIN_ECG_1, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_ecg_3_func(void * parg)
{
  cframe_command(CFRAME_ECGLEAD, (void*)MAKELONG(MAIN_ECG_3 - MAIN_ECG_1, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_ecg_4_func(void * parg)
{
  cframe_command(CFRAME_ECGLEAD, (void*)MAKELONG(MAIN_ECG_4 - MAIN_ECG_1, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_ecg_5_func(void * parg)
{
  cframe_command(CFRAME_ECGLEAD, (void*)MAKELONG(MAIN_ECG_5 - MAIN_ECG_1, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_ecg_6_func(void * parg)
{
  cframe_command(CFRAME_ECGLEAD, (void*)MAKELONG(MAIN_ECG_6 - MAIN_ECG_1, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_ecg_7_func(void * parg)
{
  cframe_command(CFRAME_ECGLEAD, (void*)MAKELONG(MAIN_ECG_7 - MAIN_ECG_1, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_ecg_func(void * parg)
{
  uframe_command(UFRAME_CREATE, (void*)MENU_ECGSET);
}

static void main_ecg_amp_func(void * parg)
{
  cframe_command(CFRAME_CHANAMP, (void*)MAKELONG(ECG, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_ecg_spd_func(void * parg)
{
  cframe_command(CFRAME_CHANSPD, (void*)MAKELONG(ECG, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_spo2_func(void * parg)
{
  uframe_command(UFRAME_CREATE, (void*)MENU_SPO2SET);
}

static void main_spo2_c_func(void * parg)
{
  main_spo2_func(parg);
}

static void main_nibp_func(void* parg)
{
  uframe_command(UFRAME_CREATE, (void*)MENU_NIBPSET);
}

static void main_resp_func(void* parg)
{
  uframe_command(UFRAME_CREATE, (void*)MENU_RESPSET);
}

static void main_t1t2_func(void* parg)
{
  uframe_command(UFRAME_CREATE, (void*)MENU_T1T2SET);
}

static void main_co2_func(void* parg)
{
  uframe_command(UFRAME_CREATE, (void*)MENU_CO2SET);
}

static void main_co2_c_func(void * parg)
{
  main_co2_func(parg);
}

static void main_co2_spd_func(void * parg)
{
  cframe_command(CFRAME_CHANSPD, (void*)MAKELONG(CO2, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_co2_amp_func(void * parg)
{
  cframe_command(CFRAME_CHANAMP, (void*)MAKELONG(CO2, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_spo2_amp_func(void * parg)
{
  cframe_command(CFRAME_CHANAMP, (void*)MAKELONG(SPO2, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_spo2_spd_func(void * parg)
{
  cframe_command(CFRAME_CHANSPD, (void*)MAKELONG(SPO2, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_resp_c_func(void * parg)
{
  main_resp_func(parg);
}

static void main_resp_spd_func(void * parg)
{
  cframe_command(CFRAME_CHANSPD, (void*)MAKELONG(RESP, ((inpfoc_cmd_t*)parg)->delta));
}


static void main_resp_amp_func(void * parg)
{
  cframe_command(CFRAME_CHANAMP, (void*)MAKELONG(RESP, ((inpfoc_cmd_t*)parg)->delta));
}

static void main_mode_func(void * parg)
{
  cframe_command(CFRAME_FILT_MODE_ROLL, (void*)((inpfoc_cmd_t*)parg)->delta);
}


