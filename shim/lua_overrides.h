#ifndef LUA_OVERRIDES_H
#define LUA_OVERRIDES_H
#include "serial.h"
#define lua_writestring(s,l) serial_write(s,l)
#define lua_writeline() serial_write("\n",1)
#define lua_writestringerror(s,p) serial_puts(s)
#endif
