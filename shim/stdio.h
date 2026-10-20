#ifndef _STDIO_H
#define _STDIO_H
#include "libc_shim.h"
typedef struct _FILE FILE;
#define stdout ((FILE*)0)
#define stderr ((FILE*)0)
#define EOF (-1)
int fflush(void *f);
size_t fwrite(const void *ptr,size_t size,size_t nmemb,void *stream);
int fputs(const char *s,void *stream);
int fprintf(void *stream,const char *fmt,...);
int printf(const char *fmt,...);
#endif

void *fopen(const char *p,const char *m);
int fclose(void *f);
size_t fread(void *ptr,size_t sz,size_t nmemb,void *stream);
int ferror(void *stream);
int feof(void *stream);
int fseek(void *stream,long off,int whence);
long ftell(void *stream);


void *freopen(const char *p,const char *m,void *stream);


int getc(void *stream);
#define stdin ((void*)0)


#define BUFSIZ 8192


char *fgets(char *s,int size,void *stream);


void *tmpfile(void);
int ungetc(int c,void *stream);
void clearerr(void *stream);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define _IONBF 2
#define _IOFBF 0
#define _IOLBF 1


int setvbuf(void *stream,char *buf,int mode,size_t size);

