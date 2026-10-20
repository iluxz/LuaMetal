#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "serial.h"

static void *my_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud; (void)osize;
    if (nsize == 0) return NULL;
    /* use libc malloc */
    extern void *malloc(size_t);
    extern void *realloc(void*,size_t);
    if (!ptr) return malloc(nsize);
    return realloc(ptr, nsize);
}

static int l_print_serial(lua_State *L) {
    int n = lua_gettop(L);
    for (int i = 1; i <= n; i++) {
        size_t l;
        const char *s = lua_tolstring(L, i, &l);
        if (!s) s = lua_typename(L, lua_type(L, i));
        if (!l) l = strlen(s);
        serial_write(s, l);
        if (i < n) serial_write("\t", 1);
    }
    serial_write("\n", 1);
    return 0;
}

void lua_run(void) {
    serial_puts("[lua] creating state\n");
    lua_State *L = lua_newstate(my_alloc, NULL);
    serial_puts("[lua] newstate returned\n");
    if (!L) { serial_puts("[lua] failed to create state\n"); return; }
    serial_puts("[lua] opening libs\n");
    luaL_openlibs(L);
    serial_puts("[lua] libs opened\n");
    lua_pushcfunction(L, l_print_serial);
    lua_setglobal(L, "print");
    serial_puts("[lua] running script\n");
    const char *script =
        "print('hello from lua on bare metal!')\n"
        "for i=1,5 do print('count', i) end\n"
        "print('2+2 =', 2+2)\n";
    if (luaL_dostring(L, script) != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        serial_puts("[lua] error: ");
        serial_puts(err ? err : "(unknown)");
        serial_puts("\n");
    } else {
        serial_puts("[lua] script done\n");
    }
    lua_close(L);
    serial_puts("[lua] closed\n");
}
