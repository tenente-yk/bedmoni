#include <stdlib.h>
#include <string.h>
#include "strtok_r.h"

#ifndef HAVE_STRTOK_R
char * strtok_r(char *s1, const char *s2, char **lasts)
{
   char *ret;
 
   if (s1 == NULL)
     s1 = *lasts;
   while(*s1 && strchr(s2, *s1))
     ++s1;
   if(*s1 == '\0')
     return NULL;
   ret = s1;
   while(*s1 && !strchr(s2, *s1))
     ++s1;
   if(*s1)
     *s1++ = '\0';
   *lasts = s1;
   return ret;
}
#if 0
char *strtok_r(char *s, const char *delim, char **save_ptr) {
    char *token;

    if (s == NULL) s = *save_ptr;

    /* Scan leading delimiters.  */
    s += strspn(s, delim);
    if (*s == '\0') return NULL;

    /* Find the end of the token.  */
    token = s;
    s = strpbrk(token, delim);
    if (s == NULL)
        /* This token finishes the string.  */
        *save_ptr = strchr(token, '\0');
    else {
        /* Terminate the token and make *SAVE_PTR point past it.  */
        *s = '\0';
        *save_ptr = s + 1;
    }

    return token;
}
#endif
#endif // HAVE_STRTOK_R
