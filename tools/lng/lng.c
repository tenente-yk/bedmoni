#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#ifdef WIN32
#include "strtok_r.h"
#endif
#include "lng.h"

#define MAX_NUM_DEFINES      3000

static struct
{
  char defname[120];
  int  value;
} defs[MAX_NUM_DEFINES];

static int num_defs = 0;

static FILE *fi, *fo;

/****************************************************************************/
static void debug(char *fmt, ...)
{
#if DEBUG
  va_list  args;
  fprintf(stdout, "debug: ");
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end( args );
#endif
}
/****************************************************************************/
static void error(char *fmt, ...)
{
  va_list  args;
  fprintf(stdout, "error: ");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end( args );
}
/****************************************************************************/
static void print_howto(void)
{
  fprintf(stdout, "lng [LNG file]\n");
}
/****************************************************************************/
static void parse_header(char *sh)
{
  FILE * f;
  char s[200], s0[255], s1[100];
  char *ps;
  char * token;
  char tmpstr[255];
  char seps[20];
  int i;

  ps = sh;
  while (*ps == ' ' || *ps == '\t' || *ps == '\"' || *ps == '<') ps ++;
  memmove(sh, ps, strlen(ps)+1); // +1 in order to copy null-terminated symbol
  i = strlen(sh)-1;
  while ( sh[i] == ' ' || sh[i] == '\t' || sh[i] == '\r' || sh[i] == '\n' || sh[i] == '\"' || sh[i] == '>')
  {
    sh[i] = 0;
    i --;
  }
//  debug("opening %s\n", sh);
  f = fopen(sh, "rt");
  if (!f)
  {
   // error("unable to open header: %s\n", sh+1);
    error("unable to open header: %s\n", sh);
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
/****************************************************************************/
int main(int argc, char * argv[])
{
  unsigned long offs;
  char sno[100], s[200], s2[200], *ps, *ps2;
  unsigned short i, ino;

//  parse_header("Debug\\1.h");

  if (argc == 1)
  {
    print_howto();
    return -1;
  }

  offs = strstr(argv[1], ".lng") - argv[1];
  if ((long)offs < 0)
  {
    fprintf(stderr, "%s: invalid file extension\n", argv[1]);
    return 1;
  }

 // parse_header("ids.h");

  strcpy(s, argv[1]);
  strcpy(s+offs, ".lang");

  fi = fopen(argv[1], "rt");
  if (!fi)
  {
    fprintf(stderr, "Unable to open %s\n", argv[1]);
    return 2;
  }

  fo = fopen(s, "wb");
  if (!fo)
  {
    fprintf(stderr, "Unable to create %s\n", s);
    return 2;
  }

  while (!feof(fi))
  {
    fgets(s, 200, fi);

    i = 0;
    while (s[i] == ' ' || s[i] == '\t')
    {
      i++;
      if (s[i] == 0) break;
    }

    if (strstr(&s[i], "#include"))
    {
      parse_header(&s[i] + strlen("#include"));
      continue;
    }
#if 0
    else
    if (isdigit(s[i]))
    {
      sscanf(s, "%d ", &ino);
      while (isdigit(s[i]) || s[i] == ' ' || s[i] == '\t')
      {
        i++;
        if (s[i] == 0) break;
      }
    }
#endif
    else
    {
      int j;
      ps = sno;

      while (s[i] != ' ' && s[i] != '\t')
      {
        *ps++ = s[i];
        i++;
        if (s[i] == 0) break;
      }
      *ps = 0;
    //  printf("sno = %s\n", sno);
      while (s[i] == ' ' || s[i] == '\t' || s[i] == '\"')
      {
        i++;
        if (s[i] == 0) break;
        if (s[i] == '\"')
        {
          i ++;
          break;
        }
      }
      ino = -1;
      for (j=0; j<num_defs; j++)
      {
        if ( strcmp(defs[j].defname, sno) == 0 )
        {
          ino = defs[j].value;
          break;
        }
      }
    }

    if ((short)ino < 0) continue;

    memmove(s, &s[i], strlen(&s[i])+1); // +1 in order to copy null-terminating symbol

    i = strlen(s)-1;
    while ( s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n' || s[i] == '\"')
    {
      if (s[i] == '\"')
      {
        s[i] = 0;
        break;
      }
      s[i] = 0;
      i --;
    }

    memset(&s[strlen(s)], 0, sizeof(s) - strlen(s));
    memset(s2, 0, sizeof(s2));

    fseek(fo, 128*ino, SEEK_SET);
    fwrite(&ino, 1, 2, fo);
    for(ps=s,ps2=s2;*ps;)
    {
      if (*ps != '\\') *ps2 = *ps;
      else
      {
        int v;
        ps++;
        sscanf(ps,"%d",&v);
    //    printf("v=%d\n",v);
        *ps2 = v;
        while(isdigit(*ps)) ps++;
        ps--;
      }
      ps++;
      ps2++;
    }
    *ps2 = 0;
//    printf("no = %d, s = %s, s2 = %s\n", ino, s, s2);
    fwrite(s2, 1, 128-2, fo);
  }

  return 0;
}
