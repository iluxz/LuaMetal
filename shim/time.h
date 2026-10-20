#ifndef _TIME_H
#define _TIME_H
#include <stddef.h>
typedef long time_t;
typedef long clock_t;
struct tm { int tm_sec,tm_min,tm_hour,tm_mday,tm_mon,tm_year,tm_wday,tm_yday,tm_isdst; };
time_t time(time_t *t);
clock_t clock(void);
struct tm *localtime(const time_t *t);
struct tm *gmtime(const time_t *t);
size_t strftime(char *s,size_t max,const char *fmt,const struct tm *tm);
double difftime(time_t t1,time_t t0);
#endif

#define L_tmpnam 256

time_t mktime(struct tm *tm);

#define CLOCKS_PER_SEC 1000

