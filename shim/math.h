#ifndef _MATH_H
#define _MATH_H
double sqrt(double x);
double floor(double x);
double ceil(double x);
double fabs(double x);
double pow(double a,double b);
double fmod(double a,double b);
double log(double x);
double exp(double x);
double sin(double x);
double cos(double x);
double tan(double x);
double asin(double x);
double acos(double x);
double atan(double x);
double atan2(double y,double x);
double modf(double x,double *iptr);
#endif

double ldexp(double x,int exp);

double frexp(double x,int *exp);
#define HUGE_VAL 1e999

double log10(double x);
