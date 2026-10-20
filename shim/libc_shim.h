#ifndef LIBC_SHIM_H
#define LIBC_SHIM_H
#include <stddef.h>
#include <stdint.h>

int isdigit(int c);
int isspace(int c);
int isalpha(int c);
int isalnum(int c);
int isxdigit(int c);
int isupper(int c);
int islower(int c);
int iscntrl(int c);
int isgraph(int c);
int ispunct(int c);
int tolower(int c);
int toupper(int c);

size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
int strcoll(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strcat(char *dst, const char *src);
char *strchr(const char *s, int c);
char *strstr(const char *hay, const char *needle);
char *strpbrk(const char *s, const char *accept);
size_t strspn(const char *s, const char *accept);
char *strrchr(const char *s, int c);

void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memchr(const void *s, int c, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);

void *malloc(size_t n);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *p, size_t n);
void free(void *p);

double strtod(const char *s, char **endptr);
long strtol(const char *s, char **endptr, int base);
unsigned long strtoul(const char *s, char **endptr, int base);

int snprintf(char *buf, size_t size, const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);

typedef struct { unsigned long regs[8]; } _jmp_buf_struct;
typedef _jmp_buf_struct jmp_buf[1];
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

void abort(void);
int abs(int x);
int system(const char *cmd);
int remove(const char *path);
int rename(const char *old,const char *nw);
char *tmpnam(char *s);
char *strerror(int errnum);

#endif



