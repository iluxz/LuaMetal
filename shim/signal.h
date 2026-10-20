#ifndef _SIGNAL_H
#define _SIGNAL_H
typedef int sig_atomic_t;
#define SIGINT 2
#define SIGTERM 15
typedef void (*sighandler_t)(int);
sighandler_t signal(int sig, sighandler_t handler);
int raise(int sig);
#endif
