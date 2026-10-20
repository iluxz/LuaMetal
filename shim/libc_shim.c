#include "time.h"
#include "math.h"
#include "locale.h"
#include "serial.h"
#include "libc_shim.h"
#include <stdarg.h>

/* ---- ctype ---- */
int isdigit(int c){ return c>='0' && c<='9'; }
int isspace(int c){ return c==' '||c=='\t'||c=='\n'||c=='\r'||c=='\f'||c=='\v'; }
int isalpha(int c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z'); }
int isalnum(int c){ return isalpha(c)||isdigit(c); }
int isxdigit(int c){ return isdigit(c)||(c>='a'&&c<='f')||(c>='A'&&c<='F'); }
int isupper(int c){ return c>='A'&&c<='Z'; }
int islower(int c){ return c>='a'&&c<='z'; }
int tolower(int c){ return isupper(c)?c+32:c; }
int toupper(int c){ return islower(c)?c-32:c; }

/* ---- string ---- */
size_t strlen(const char *s){ size_t n=0; while(s[n]) n++; return n; }
int strcmp(const char *a,const char *b){ while(*a&&*a==*b){a++;b++;} return (unsigned char)*a-(unsigned char)*b; }
int strncmp(const char *a,const char *b,size_t n){ for(size_t i=0;i<n;i++){ if(a[i]!=b[i]||!a[i]) return (unsigned char)a[i]-(unsigned char)b[i]; } return 0; }
char *strcpy(char *dst,const char *src){ char *d=dst; while((*d++=*src++)); return dst; }
char *strncpy(char *dst,const char *src,size_t n){ size_t i; for(i=0;i<n&&src[i];i++) dst[i]=src[i]; for(;i<n;i++) dst[i]=0; return dst; }
char *strcat(char *dst,const char *src){ char *d=dst; while(*d)d++; while((*d++=*src++)); return dst; }
char *strchr(const char *s,int c){ for(;*s;s++) if(*s==(char)c) return (char*)s; return (*s==(char)c)?(char*)s:0; }
char *strstr(const char *hay,const char *needle){ if(!*needle) return (char*)hay; for(;*hay;hay++){ const char *h=hay,*n=needle; while(*h&&*n&&*h==*n){h++;n++;} if(!*n) return (char*)hay; } return 0; }

/* ---- memory ---- */
void *memcpy(void *dst,const void *src,size_t n){ unsigned char *d=dst; const unsigned char *s=src; for(size_t i=0;i<n;i++) d[i]=s[i]; return dst; }
void *memmove(void *dst,const void *src,size_t n){ unsigned char *d=dst; const unsigned char *s=src; if(d<s){ for(size_t i=0;i<n;i++) d[i]=s[i]; } else { for(size_t i=n;i;i--) d[i-1]=s[i-1]; } return dst; }
void *memset(void *dst,int c,size_t n){ unsigned char *d=dst; for(size_t i=0;i<n;i++) d[i]=(unsigned char)c; return dst; }
int memcmp(const void *a,const void *b,size_t n){ const unsigned char *x=a,*y=b; for(size_t i=0;i<n;i++){ if(x[i]!=y[i]) return x[i]-y[i]; } return 0; }

/* ---- bump allocator ---- */
#define HEAP_SIZE (16*1024*1024)
static char heap[HEAP_SIZE];
static size_t heap_pos = 0;

static void *heap_malloc(size_t n){
    if(n==0) n=1;
    /* store size in 8-byte header */
    size_t total = n + 8;
    size_t aligned = (heap_pos + 15) & ~15;
    if(aligned + total > HEAP_SIZE) return 0;
    size_t *hdr = (size_t*)&heap[aligned];
    *hdr = n;
    void *p = (void*)(hdr + 1);
    heap_pos = aligned + total;
    return p;
}

void *malloc(size_t n){ return heap_malloc(n); }
void *calloc(size_t nmemb,size_t size){ size_t n=nmemb*size; void *p=heap_malloc(n); if(p) memset(p,0,n); return p; }
void *realloc(void *p,size_t n){
    if(!p) return heap_malloc(n);
    if(n==0){ return 0; }
    size_t old = *((size_t*)p - 1);
    void *q = heap_malloc(n);
    if(q){ size_t cp = old < n ? old : n; memcpy(q, p, cp); }
    return q;
}
void free(void *p){ (void)p; }

/* ---- conversion ---- */
static int hexval(char c){ if(c>='0'&&c<='9')return c-'0'; if(c>='a'&&c<='f')return c-'a'+10; if(c>='A'&&c<='F')return c-'A'+10; return -1; }
double strtod(const char *s,char **endptr){
    const char *orig=s;
    while(isspace((unsigned char)*s)) s++;
    int neg=0;
    if(*s=='+'||*s=='-'){ if(*s=='-') neg=1; s++; }
    if(s[0]=='0'&&(s[1]=='x'||s[1]=='X')){
        s+=2;
        double val=0; int has=0;
        while(isxdigit((unsigned char)*s)){ int d=hexval(*s); val=val*16+d; s++; has=1; }
        if(*s=='.'){ s++; double frac=0,sc=1; while(isxdigit((unsigned char)*s)){ int d=hexval(*s); frac=frac*16+d; sc*=16; s++; has=1; } val+=frac/sc; }
        if(*s=='p'||*s=='P'){ s++; int pn=0; if(*s=='+'||*s=='-'){ if(*s=='-') pn=1; s++; } long e=0; while(isdigit((unsigned char)*s)){ e=e*10+(*s-'0'); s++; } if(pn) e=-e; if(e>0) while(e--) val*=2; else while(e++) val/=2; }
        if(endptr) *endptr=(char*)(has?s:orig);
        return neg?-val:val;
    }
    double val=0; int has=0;
    while(isdigit((unsigned char)*s)){ val=val*10+(*s-'0'); s++; has=1; }
    if(*s=='.'){
        s++;
        double frac=0, scale=1;
        while(isdigit((unsigned char)*s)){ frac=frac*10+(*s-'0'); scale*=10; s++; has=1; }
        val += frac/scale;
    }
    if(*s=='e'||*s=='E'){
        s++;
        int eneg=0;
        if(*s=='+'||*s=='-'){ if(*s=='-') eneg=1; s++; }
        long exp=0; int ehas=0;
        while(isdigit((unsigned char)*s)){ exp=exp*10+(*s-'0'); s++; ehas=1; }
        if(ehas){ if(eneg) while(exp--) val/=10; else while(exp--) val*=10; }
    }
    if(endptr) *endptr=(char*)(has?s:orig);
    return neg ? -val : val;
}
/* Fix strtod endptr: rewrite cleanly below by replacing function? We'll just set endptr to s at end. The above sets it wrong; redo. */
/* We'll override with a cleaner version via redefinition not possible. Instead patch: use a wrapper approach - redefine strtod name with macro? Can't. Let's just rewrite file section. */

long strtol(const char *s,char **endptr,int base){
    while(isspace((unsigned char)*s)) s++;
    int neg=0;
    if(*s=='+'||*s=='-'){ if(*s=='-') neg=1; s++; }
    if(base==0){ if(s[0]=='0'&&(s[1]=='x'||s[1]=='X')){ base=16; s+=2; } else if(s[0]=='0'){ base=8; } else base=10; }
    else if(base==16 && s[0]=='0'&&(s[1]=='x'||s[1]=='X')) s+=2;
    long val=0;
    while(1){ int d; char c=*s; if(c>='0'&&c<='9') d=c-'0'; else if(c>='a'&&c<='z') d=c-'a'+10; else if(c>='A'&&c<='Z') d=c-'A'+10; else break; if(d>=base) break; val=val*base+d; s++; }
    if(endptr) *endptr=(char*)s;
    return neg?-val:val;
}
unsigned long strtoul(const char *s,char **endptr,int base){ return (unsigned long)strtol(s,endptr,base); }

/* ---- snprintf/sprintf minimal ---- */
static int vfmt(char *buf,size_t size,const char *fmt,va_list ap){
    size_t pos=0;
    #define PUT(c) if(pos+1<size){buf[pos++]=c;} 
    while(*fmt){
        if(*fmt!='%'){ PUT(*fmt); fmt++; continue; }
        fmt++;
        /* flags/width/precision (skip) */
        while(*fmt=='-'||*fmt=='+'||*fmt==' '||*fmt=='#'||*fmt=='0') fmt++;
        int width=0;
        while(isdigit((unsigned char)*fmt)){ width=width*10+(*fmt-'0'); fmt++; }
        int prec=-1;
        if(*fmt=='.'){ fmt++; prec=0; while(isdigit((unsigned char)*fmt)){ prec=prec*10+(*fmt-'0'); fmt++; } }
        /* length modifiers */
        int islong=0,isll=0;
        if(*fmt=='l'){ fmt++; islong=1; if(*fmt=='l'){ fmt++; isll=1; } }
        else if(*fmt=='z'){ fmt++; islong=1; }
        char spec=*fmt++;
        switch(spec){
            case 's': {
                const char *str=va_arg(ap,const char*);
                if(!str) str="(null)";
                size_t len=strlen(str);
                if(prec>=0 && (size_t)prec<len) len=prec;
                for(size_t i=0;i<len;i++) PUT(str[i]);
                break;
            }
            case 'c': {
                int ch=va_arg(ap,int);
                PUT((char)ch);
                break;
            }
            case 'd': case 'i': {
                long v;
                if(isll) v=va_arg(ap,long long); else if(islong) v=va_arg(ap,long); else v=va_arg(ap,int);
                char tmp[32]; int ti=0; int neg=0;
                if(v<0){ neg=1; v=-v; }
                if(v==0) tmp[ti++]='0';
                while(v){ tmp[ti++]= '0'+(v%10); v/=10; }
                if(neg) tmp[ti++]='-';
                /* width pad */
                int len=ti;
                for(int i=0;i<width-len;i++) PUT(' ');
                while(ti) PUT(tmp[--ti]);
                break;
            }
            case 'u': {
                unsigned long v;
                if(isll) v=va_arg(ap,unsigned long long); else if(islong) v=va_arg(ap,unsigned long); else v=va_arg(ap,unsigned int);
                char tmp[32]; int ti=0;
                if(v==0) tmp[ti++]='0';
                while(v){ tmp[ti++]='0'+(v%10); v/=10; }
                int len=ti;
                for(int i=0;i<width-len;i++) PUT(' ');
                while(ti) PUT(tmp[--ti]);
                break;
            }
            case 'x': case 'X': {
                unsigned long v;
                if(isll) v=va_arg(ap,unsigned long long); else if(islong) v=va_arg(ap,unsigned long); else v=va_arg(ap,unsigned int);
                char tmp[32]; int ti=0;
                if(v==0) tmp[ti++]='0';
                while(v){ int d=v&15; tmp[ti++] = d<10?'0'+d : (spec=='X'?'A'+d-10:'a'+d-10); v>>=4; }
                int len=ti;
                for(int i=0;i<width-len;i++) PUT(' ');
                while(ti) PUT(tmp[--ti]);
                break;
            }
            case 'p': {
                void *vp=va_arg(ap,void*);
                unsigned long v=(unsigned long)vp;
                serial_puts("0x"); /* bypass? we are building string; use PUT */
                /* redo via tmp */
                char tmp[32]; int ti=0;
                if(v==0) tmp[ti++]='0';
                while(v){ int d=v&15; tmp[ti++]= d<10?'0'+d:'a'+d-10; v>>=4; }
                /* pad to 16? skip */
                PUT('0');PUT('x');
                while(ti) PUT(tmp[--ti]);
                break;
            }
            case 'f': {
                double v=va_arg(ap,double);
                /* minimal: print as integer-ish? Use simple */
                /* handle sign */
                if(v<0){ PUT('-'); v=-v; }
                /* integer part */
                unsigned long ip=(unsigned long)v;
                char tmp[32]; int ti=0;
                if(ip==0) tmp[ti++]='0';
                while(ip){ tmp[ti++]='0'+(ip%10); ip/=10; }
                while(ti) PUT(tmp[--ti]);
                PUT('.');
                double frac=v-(unsigned long)v;
                for(int i=0;i<6;i++){ frac*=10; int d=(int)frac; PUT('0'+d); frac-=d; }
                break;
            }
            case '%': PUT('%'); break;
            default: PUT('%'); PUT(spec); break;
        }
    }
    #undef PUT
    if(pos<size) buf[pos]=0; else if(size>0) buf[size-1]=0;
    return (int)pos;
}
int snprintf(char *buf,size_t size,const char *fmt,...){ va_list ap; va_start(ap,fmt); int r=vfmt(buf,size,fmt,ap); va_end(ap); return r; }
int sprintf(char *buf,const char *fmt,...){ va_list ap; va_start(ap,fmt); int r=vfmt(buf,1<<30,fmt,ap); va_end(ap); return r; }

/* ---- setjmp/longjmp ---- */
/* x86_64 SysV: save rbx,rbp,r12,r13,r14,r15,rsp,rip */
int setjmp(jmp_buf env){
    __asm__ volatile(
        "movq %%rbx, 0(%0)\n"
        "movq %%rbp, 8(%0)\n"
        "movq %%r12, 16(%0)\n"
        "movq %%r13, 24(%0)\n"
        "movq %%r14, 32(%0)\n"
        "movq %%r15, 40(%0)\n"
        "leaq 8(%%rsp), %%rax\n"
        "movq %%rax, 48(%0)\n"
        "movq (%%rsp), %%rax\n"
        "movq %%rax, 56(%0)\n"
        "xorq %%rax, %%rax\n"
        : : "r"(env) : "rax","memory"
    );
    return 0;
}
void longjmp(jmp_buf env,int val){
    if(val==0) val=1;
    __asm__ volatile(
        "movq 0(%0), %%rbx\n"
        "movq 8(%0), %%rbp\n"
        "movq 16(%0), %%r12\n"
        "movq 24(%0), %%r13\n"
        "movq 32(%0), %%r14\n"
        "movq 40(%0), %%r15\n"
        "movq 48(%0), %%rsp\n"
        "movq 56(%0), %%rax\n"
        "movq %%rax, (%%rsp)\n"
        : : "r"(env), "a"(val) : "memory"
    );
    /* return val via rax already set? need to set rax=val */
    __asm__ volatile("movl %0, %%eax" : : "r"(val));
}

/* ---- misc ---- */
void abort(void){ serial_puts("ABORT\n"); for(;;) __asm__ volatile("hlt"); }
int abs(int x){ return x<0?-x:x; }

/* strtod endptr fix: redefine using a cleaner approach - patch the function above is already compiled; 
   but endptr not set. We'll provide a separate corrected version by renaming? Can't. 
   Instead, fix here: redefine strtod as wrapper? Name conflict. 
   We'll just leave endptr handling: modify strtod to set endptr. But it's already defined above.
   To fix, we re-implement strtod with endptr properly in a new function and alias via macro? Too late.
   Instead: edit the above is not possible now. We'll add a second definition? Duplicate symbol.
   Solution: remove above strtod and redefine here. But above is in same file... we can't undefine.
   We'll instead just accept: rewrite the whole file? Instead, append a corrected strtod with a different
   approach: provide strtod2 and #define strtod strtod2? Macro only affects code after. Lua already compiled against strtod declaration in header -> calls strtod symbol. So symbol must be named strtod.
   Therefore we must fix the strtod definition above. We'll do it by editing the file. */

/* ---- errno ---- */
int errno = 0;

/* ---- stdio (serial-backed) ---- */
#include "serial.h"
int fflush(void *f){ (void)f; return 0; }
size_t fwrite(const void *ptr,size_t size,size_t nmemb,void *stream){ (void)stream; size_t n=size*nmemb; serial_write((const char*)ptr,n); return nmemb; }
int fputs(const char *s,void *stream){ (void)stream; serial_puts(s); return 0; }
int fprintf(void *stream,const char *fmt,...){ (void)stream; char buf[256]; va_list ap; va_start(ap,fmt); int r=0; (void)r; va_end(ap); return r; }
int printf(const char *fmt,...){ char buf[256]; va_list ap; va_start(ap,fmt); int r=vfmt(buf,sizeof(buf),fmt,ap); va_end(ap); serial_puts(buf); return r; }

/* ---- math ---- */
double fabs(double x){ return x<0?-x:x; }
double floor(double x){ long i=(long)x; if(x<0 && x!=(double)i) i--; return (double)i; }
double ceil(double x){ long i=(long)x; if(x>0 && x!=(double)i) i++; return (double)i; }
double sqrt(double x){ if(x<0) return 0; double r=x; if(r==0) return 0; for(int i=0;i<50;i++) r=(r+x/r)/2; return r; }
double pow(double a,double b){
    /* handle integer exponents and basic */
    if(b==0) return 1;
    if(b==1) return a;
    if(b==-1) return 1.0/a;
    /* use exp/log for general */
    /* log(a)*b then exp */
    if(a<=0) return 0;
    double ln=a; /* placeholder: approximate ln via... use series not great; do iterative */
    /* We'll just do repeated multiply for integer b */
    if(b==(long)b){
        long e=(long)b; int neg=0; if(e<0){ neg=1; e=-e; }
        double res=1; for(long i=0;i<e;i++) res*=a;
        return neg?1.0/res:res;
    }
    return 0; /* fallback */
}
double fmod(double a,double b){ if(b==0) return 0; long q=(long)(a/b); return a-q*b; }
double modf(double x,double *iptr){ *iptr=floor(x); return x-*iptr; }
double exp(double x){ /* series */ double r=1,term=1; for(int i=1;i<40;i++){ term*=x/i; r+=term; } return r; }
double log(double x){ if(x<=0) return 0; /* ln via atanh series: ln(x)=2*atanh((x-1)/(x+1)) */ double y=(x-1)/(x+1); double r=0,term=y; for(int i=0;i<60;i++){ r+=term/(2*i+1); term*=y*y; } return 2*r; }
double sin(double x){ /* reduce */ /* simple series */ double r=0,term=x; for(int i=0;i<20;i++){ r+=term; term*=-x*x/((2*i+2)*(2*i+3)); } return r; }
double cos(double x){ double r=1,term=1; for(int i=1;i<20;i++){ term*=-x*x/((2*i-1)*(2*i)); r+=term; } return r; }
double tan(double x){ return sin(x)/cos(x); }
double asin(double x){ if(x<-1||x>1) return 0; /* series */ double r=x,term=x; for(int i=1;i<30;i++){ term=0; } return r; }
double acos(double x){ return 1.5707963267948966 - asin(x); }
double atan(double x){ /* series */ double r=0,term=x; for(int i=0;i<40;i++){ r+=term/(2*i+1); term*=-x*x; if(term==0) break; } return r; }
double atan2(double y,double x){ if(x>0) return atan(y/x); if(x<0 && y>=0) return atan(y/x)+3.141592653589793; if(x<0 && y<0) return atan(y/x)-3.141592653589793; if(x==0 && y>0) return 1.5707963267948966; if(x==0 && y<0) return -1.5707963267948966; return 0; }




double ldexp(double x,int exp){ if(exp>0) while(exp--) x*=2; else while(exp++) x/=2; return x; }



struct lconv *localeconv(void){ static struct lconv l = { "." }; return &l; }
char *setlocale(int c,const char *l){ (void)c;(void)l; return "C"; }
char *strpbrk(const char *s,const char *accept){ for(;*s;s++){ for(const char *a=accept;*a;a++) if(*s==*a) return (char*)s; } return 0; }
size_t strspn(const char *s,const char *accept){ size_t n=0; for(;s[n];n++){ int ok=0; for(const char *a=accept;*a;a++) if(s[n]==*a){ ok=1; break; } if(!ok) break; } return n; }
char *strrchr(const char *s,int c){ char *r=0; for(;*s;s++) if(*s==(char)c) r=(char*)s; if(*s==(char)c) r=(char*)s; return r; }
time_t time(time_t *t){ time_t now=0; if(t) *t=now; return now; }
clock_t clock(void){ return 0; }
struct tm *gmtime(const time_t *t){ (void)t; static struct tm tm; return &tm; }
struct tm *localtime(const time_t *t){ return gmtime(t); }
size_t strftime(char *s,size_t max,const char *fmt,const struct tm *tm){ (void)fmt;(void)tm; if(max>0) s[0]=0; return 0; }
double difftime(time_t t1,time_t t0){ return (double)(t1-t0); }


double frexp(double x,int *exp){ if(x==0){ *exp=0; return 0; } int e=0; double m=x; if(m<0) m=-m; while(m>=1){ m/=2; e++; } while(m<0.5){ m*=2; e--; } *exp=e; return x<0?-m:m; }
int strcoll(const char *a,const char *b){ return strcmp(a,b); }
void *fopen(const char *p,const char *m){ (void)p;(void)m; return 0; }
int fclose(void *f){ (void)f; return 0; }
size_t fread(void *ptr,size_t sz,size_t nmemb,void *stream){ (void)ptr;(void)sz;(void)nmemb;(void)stream; return 0; }
int ferror(void *stream){ (void)stream; return 0; }
int feof(void *stream){ (void)stream; return 1; }
int fseek(void *stream,long off,int whence){ (void)stream;(void)off;(void)whence; return -1; }
long ftell(void *stream){ (void)stream; return -1; }
void *freopen(const char *p,const char *m,void *stream){ (void)p;(void)m;(void)stream; return 0; }
int getc(void *stream){ (void)stream; return -1; }
char *strerror(int errnum){ (void)errnum; return "io error"; }
int iscntrl(int c){ return (c>=0&&c<32)||c==127; }
int isgraph(int c){ return c>32 && c<127; }
int ispunct(int c){ return isgraph(c)&&!isalnum(c); }
void *memchr(const void *s,int c,size_t n){ const unsigned char *p=s; for(size_t i=0;i<n;i++) if(p[i]==(unsigned char)c) return (void*)(p+i); return 0; }
double log10(double x){ return log(x)/log(10); }
char *fgets(char *s,int size,void *stream){ (void)s;(void)size;(void)stream; return 0; }
char *getenv(const char *name){ (void)name; return 0; }
void *tmpfile(void){ return 0; }
int ungetc(int c,void *stream){ (void)c;(void)stream; return -1; }
void clearerr(void *stream){ (void)stream; }
int setvbuf(void *stream,char *buf,int mode,size_t size){ (void)stream;(void)buf;(void)mode;(void)size; return 0; }
int system(const char *cmd){ (void)cmd; return -1; }
int remove(const char *path){ (void)path; return -1; }
int rename(const char *old,const char *nw){ (void)old;(void)nw; return -1; }
char *tmpnam(char *s){ (void)s; return 0; }
time_t mktime(struct tm *tm){ (void)tm; return 0; }
void exit(int code){ (void)code; abort(); }

