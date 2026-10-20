#ifndef _STDLIB_H
#define _STDLIB_H
#include "libc_shim.h"
/* extras lua might need */
void abort(void);
#endif

char *getenv(const char *name);


#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


void exit(int code);

