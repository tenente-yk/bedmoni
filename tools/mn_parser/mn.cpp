#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "mn.h"
#include "strtok_r.h"

#define MAX_NUM_DEFINES      2000
//#define DEMO

static struct
{
  char defname[100];
  int  value;
} defs[MAX_NUM_DEFINES];

typedef struct
{
  char cname[50];
  int  id;
} mi_info_t;

static mi_info_t mi_info[] =
{
  {"STATIC",      MI_STATIC},
  {"INPFOC",      MI_INPFOC},
  {"SPINBUTTON",  MI_SPINBUTTON},
  {"CHECKBOX",    MI_CHECKBOX},
  {"SLIDER",      MI_SLIDER},
  {"PROGRESSBAR", MI_PROGRESSBAR},
  {"LISTBUTTON",  MI_LISTBUTTON},
  {"RADIOBUTTON", MI_RADIOBUTTON},
};

static int num_defs = 0;
//int verbose = 0;

void debug(char *fmt, ...)
{
#if DEBUG
  fprintf(stdout, "debug: ");
  va_list  args;
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end( args );
#endif
}

void error(char *fmt, ...)
{
  fprintf(stdout, "error: ");
  va_list  args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end( args );
}


void def_proc(char * def)
{
  int i;

  for (i=0; i<num_defs; i++)
  {
    if (strcmp(defs[i].defname, def) == 0)
    {
      char sv[10];
      sprintf(sv, "%d", defs[i].value);
      strcpy(def, sv);
      break;
    }
  }

}

void parse_header(char *sh)
{
  FILE * f;
  char s[255], s0[255], s1[100];
  char *ps;
  char * token;
  char tmpstr[255];
  char seps[20];
  int i;

  sh[strlen(sh)-2] = 0;
  debug("opening %s\n", sh+1);
  f = fopen(sh+1, "rt");
  if (!f)
  {
    error("unable to open header: %s\n", sh+1);
    return;
  }

  while (!feof(f))
  {
    fgets(s, sizeof(s), f);
    //debug(s);

#if 0
    char * ps;
    ps = strstr(s, "#include");
    if (ps)
    {
      ps += strlen("#include");
      while ((*ps == ' ') || (*ps == '\t')) ps ++;
      parse_header(ps);
    }
#endif

#ifdef WIN32
    _snprintf(seps, sizeof(seps), " \t");
#endif
#ifdef UNIX
    snprintf(seps, sizeof(seps), " \t");
#endif
    strcpy(s0, s);
    token = strtok_r(s0, seps, (char**)&tmpstr);
    if (token)
    {
      if (strcmp(token, "#define") == 0)
      {
        token = strtok_r(NULL, seps, (char**)&tmpstr);
        sprintf(defs[num_defs].defname, token);
        token = strtok_r(NULL, seps, (char**)&tmpstr);
        if (token)
        {
          defs[num_defs].value = atoi(token);
          num_defs += 1;
          if (num_defs >= MAX_NUM_DEFINES)
          {
            debug("Defines' overflow!!! Unstable behaviour expected\n");
          }
        }
        continue;
      }
      if ((strcmp(token, "enum") == 0) || (strcmp(token, "enum\n") == 0) || (strcmp(token, "enum\r\n") == 0))
      {
       // OutputDebugString("enum");
        int prevval = -1;
        while ( !feof(f) && (strstr(s,"{") == NULL) )
          fgets(s, sizeof(s), f);
        while ( !feof(f) && (strstr(s,"}") == NULL) )
        {
        //  static int prevval = -1;
          fgets(s, sizeof(s), f);

          i = 0;
          while (s[i] == ' ' || s[i] == '\t') i++;
          *s1 = 0;
          ps = s1;
          while (s[i] != ' ' && s[i] != '\t' && s[i] != 0 && s[i] != ',') { *ps++ = s[i]; i++; }
          *ps = 0;
         // token = strtok_r(s, seps, (char**)&tmpstr);
          sprintf(defs[num_defs].defname, s1);
          *s1 = 0;
          ps = s1;
          while (s[i] == ' ' || s[i] == '\t') i++;
          while (s[i] != ' ' && s[i] != '\t' && s[i] != 0) { *ps++ = s[i]; i++; }
          *ps = 0;
          if (*s1 == ',')
          {
            defs[num_defs].value = prevval+1;
            prevval ++;
            debug("%s %d\n", defs[num_defs].defname, defs[num_defs].value);
            num_defs ++;
          }
          if (*s1 == '=')
          {
            *s1 = 0;
            ps = s1;
            while (s[i] == ' ' || s[i] == '\t') i++;
            if (!isdigit(s[i]))
            {
              defs[num_defs].value = prevval;
              debug("%s %d\n", defs[num_defs].defname, defs[num_defs].value);
              num_defs ++;
            }
            else
            {
              while (s[i] != ' ' && s[i] != '\t' && s[i] != 0 && s[i] != ',') { *ps++ = s[i]; i++; }
              *ps = 0;
              defs[num_defs].value = atoi(s1);
              prevval = defs[num_defs].value;
              debug("%s %d\n", defs[num_defs].defname, defs[num_defs].value);
              num_defs ++;
            }
          }
        }
      }
    }
  }

  fclose(f);

}

static void decode_static(char *s, FILE * fo)
{
  char *ps, *pso, *pst;
  char outstr[200], tmpstr[255];
  unsigned long v;

  ps = s;
  pso = outstr;

  *pso++ = MI_BEG;
  *pso++ = 0x00;      // packet len (be filled below)
  *pso++ = 0x00;      // packet len (be filled below)

  *pso++ = MI_STATIC; // type

  ps += strlen("STATIC");

  // retrieve caption id
  while (*ps == ' ' || *ps == '\t' || *ps == '\"') ps ++;
  strcpy(tmpstr, ps);
  if (strncmp(tmpstr, "IDS_", 4) == 0)
  {
    int i;
    pst = tmpstr;
    while (*pst != ' ' && *pst != '\t' && *pst != ',') pst ++;
    *pst = 0;
    for (i=0; i<num_defs; i++)
    {
      if ( strcmp(defs[i].defname, tmpstr) == 0 )
      {
        ps += strlen(tmpstr);
        *pso++ = (defs[i].value >> 0) & 0xff;
        *pso++ = (defs[i].value >> 8) & 0xff;
        break;
      }
    }
    if (i == num_defs) return;
  }
  else
  {
    return;
  }

  *pso++ = 0x00;      // inpfoc id
  *pso++ = 0x00;      // inpfoc item

  // store x
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  sscanf(ps, "%d", &v);
  memcpy(pso, &v, 2);
  pso += 2;
  while (isdigit(*ps)) ps ++;

  // store y
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  sscanf(ps, "%d", &v);
  memcpy(pso, &v, 2);
  pso += 2;
  while (isdigit(*ps)) ps ++;

  // store cx
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  sscanf(ps, "%d", &v);
  memcpy(pso, &v, 2);
  pso += 2;
  while (isdigit(*ps)) ps ++;

  // store cy
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  sscanf(ps, "%d", &v);
  memcpy(pso, &v, 2);
  pso += 2;
  while (isdigit(*ps)) ps ++;

  *pso++ = 0x00;      // reserved
  *pso++ = 0x00;      // reserved
  *pso++ = MI_END;

  *((unsigned short*)&outstr[1]) = pso-outstr; // packet len

  fwrite(outstr, pso-outstr, 1, fo);

  fflush(fo);
}

static void decode_inpfoc(int type, char *s, FILE * fo)
{
  char *ps, *pso, *pst;
  char outstr[200], tmpstr[255];
  unsigned long v;
  int i;

  ps = s;
  pso = outstr;

  *pso++ = MI_BEG;
  *pso++ = 0x00;      // packet len (be filled below)
  *pso++ = 0x00;      // packet len (be filled below)
  *pso++ = type;      // type

  // retrieve caption id
  while (*ps == ' ' || *ps == '\t' || *ps == '\"') ps ++;
  strcpy(tmpstr, ps);
  if (strncmp(tmpstr, "IDS_", 4) == 0)
  {
    int i;
    pst = tmpstr;
    while (*pst != ' ' && *pst != '\t' && *pst != ',') pst ++;
    *pst = 0;
    for (i=0; i<num_defs; i++)
    {
      if ( strcmp(defs[i].defname, tmpstr) == 0 )
      {
        ps += strlen(tmpstr);
        *pso++ = (defs[i].value >> 0) & 0xff;
        *pso++ = (defs[i].value >> 8) & 0xff;
        break;
      }
    }
    if (i == num_defs) return;
  }
  else
  {
    return;
  }

  // retrieve inpfoc id
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  strcpy(tmpstr, ps);
  pst = tmpstr;
  while (*pst != ',') pst ++;
  *pst = 0;
  v = 0;
  for (i=0; i<num_defs; i++)
  {
    if (strcmp(defs[i].defname, tmpstr) == 0)
    {
      v = defs[i].value;
      break;
    }
  }
  ps += strlen(tmpstr);
  *pso++ = (char)v;   // inpfoc id

  // retrieve inpfoc item id
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  strcpy(tmpstr, ps);
  pst = tmpstr;
  while (*pst != ',') pst ++;
  *pst = 0;
  v = 0;
  for (i=0; i<num_defs; i++)
  {
    if (strcmp(defs[i].defname, tmpstr) == 0)
    {
      v = defs[i].value;
      break;
    }
  }
  ps += strlen(tmpstr);
  *pso++ = (char)v;   // inpfoc item

  // store x
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  sscanf(ps, "%d", &v);
  memcpy(pso, &v, 2);
  pso += 2;
  while (isdigit(*ps)) ps ++;

  // store y
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  sscanf(ps, "%d", &v);
  memcpy(pso, &v, 2);
  pso += 2;
  while (isdigit(*ps)) ps ++;

  // store cx
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  sscanf(ps, "%d", &v);
  memcpy(pso, &v, 2);
  pso += 2;
  while (isdigit(*ps)) ps ++;

  // store cy
  while (*ps == ' ' || *ps == '\t' || *ps == ',' || *ps == '\"') ps ++;
  sscanf(ps, "%d", &v);
  memcpy(pso, &v, 2);
  pso += 2;
  while (isdigit(*ps)) ps ++;

  *pso++ = 0x00;      // reserved
  *pso++ = 0x00;      // reserved
  *pso++ = MI_END;

  *((unsigned short*)&outstr[1]) = pso-outstr; // packet len

  fwrite(outstr, 1, pso-outstr, fo);
}

static void mn_process(char * fname)
{
  FILE *fi, *fo;
  char s[255], s0[255];
  char * token;
  char tmpstr[255];
  char seps[20];
  char fnameo[255];
  int offs;
  int i;

  offs = strstr(fname, ".mn") - fname;
  if (offs < 0)
  {
    fprintf(stderr, "%s: invalid extension\n", fname);
    exit(-1);
  }

  strcpy(fnameo, fname);
  strcpy(fnameo+offs, ".men");

  fi = fopen(fname, "rt");
  if (!fi)
  {
    error("fopen fi error\n");
    return;
  }

  fo = fopen(fnameo, "wb");
  if (!fo)
  {
    error("fopen fo error\n");
    return;
  }
  num_defs = 0;

  while (!feof(fi))
  {
    fgets(s, sizeof(s), fi);

#ifdef WIN32
    _snprintf(seps, sizeof(seps), " \t");
#endif
#ifdef UNIX
    snprintf(seps, sizeof(seps), " \t");
#endif
    strcpy(s0, s);
    token = strtok_r(s0, seps, (char**)&tmpstr);
    if (token)
    {
      if (strcmp(token, "#include") == 0)
      {
        token = strtok_r(NULL, seps, (char**)&tmpstr);
        parse_header(token);
#ifdef WIN32
        OutputDebugString(token);
        OutputDebugString("\n");
#endif
        continue;
      }

      if ( strcmp(token, "STATIC") == 0 )
      {
        decode_static(s, fo);
        continue;
      }
      else
      {
        for (i=0; i<sizeof(mi_info)/sizeof(mi_info_t); i++)
        {
          if (strcmp(token, mi_info[i].cname) == 0)
          {
            decode_inpfoc(mi_info[i].id, s+strlen(mi_info[i].cname), fo);
          }
        }
        continue;
      }
    }
  }

  // eof
  unsigned long v;
  v = (MI_EOF<<24) | (MI_EOF<<16) | (MI_EOF<<8) | (MI_EOF<<0);
  fwrite(&v, 1, 4, fo);

  fclose(fi);
  fclose(fo);
}

int main(int argc, char* argv[])
{
#ifndef DEMO
  char fname[255];
  int i;

  if (argc == 1)
  {
    printf("usage: mn_parser [MN FILE]\n");
    exit(-1);
  }

  debug("debugging on\n");

/*  debug("argc=%d\n", argc);
  int i;
  for (i=0; i<argc; i++)
  {
    debug("argv[%d] = %s\n", i, argv[i]);
  }
*/

  for (i=1; i<argc; i++)
  {
    sprintf(fname, argv[i]);
    mn_process(fname);
  }
#else
  mn_process("general.mn");
#endif

  return 0;
}
