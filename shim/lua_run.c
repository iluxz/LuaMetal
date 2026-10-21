#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "serial.h"
#include "gfx.h"
#include "input.h"

static void *my_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud; (void)osize;
    if (nsize == 0) return NULL;
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

static int l_mx(lua_State *L) { lua_pushinteger(L, input_x()); return 1; }
static int l_my(lua_State *L) { lua_pushinteger(L, input_y()); return 1; }
static int l_btn(lua_State *L) { lua_pushinteger(L, input_btn()); return 1; }
static int l_scancode(lua_State *L) { lua_pushinteger(L, input_scancode()); return 1; }

static void input_register(lua_State *L) {
    static const luaL_Reg fns[] = {
        {"mx", l_mx}, {"my", l_my}, {"btn", l_btn}, {"scancode", l_scancode},
        {NULL, NULL}
    };
    luaL_newlib(L, fns);
    lua_setglobal(L, "input");
}

void lua_run(const char **bufs, unsigned long *lens, int nmods) {
    serial_puts("[lua] creating state\n");
    lua_State *L = lua_newstate(my_alloc, NULL);
    if (!L) { serial_puts("[lua] failed to create state\n"); return; }
    luaL_openlibs(L);
    lua_pushcfunction(L, l_print_serial);
    lua_setglobal(L, "print");
    gfx_register(L);
    input_register(L);
    serial_puts("[lua] gfx + input modules registered\n");

    for (int i = 0; i < nmods; i++) {
        serial_puts("[lua] running module\n");
        int rc = luaL_loadbufferx(L, bufs[i], lens[i], "module", "t");
        if (rc == LUA_OK) rc = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (rc != LUA_OK) {
            const char *err = lua_tostring(L, -1);
            serial_puts("[lua] error: ");
            serial_puts(err ? err : "(unknown)");
            serial_puts("\n");
            lua_pop(L, 1);
        }
    }

    /* main loop: if init.lua defined step(), poll input and run it forever */
    lua_getglobal(L, "step");
    int has_step = lua_isfunction(L, -1);
    lua_pop(L, 1);
    if (has_step) {
        serial_puts("[lua] entering step loop\n");
        for (;;) {
            input_poll();
            lua_getglobal(L, "step");
            if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                const char *err = lua_tostring(L, -1);
                serial_puts("[lua] step error: ");
                serial_puts(err ? err : "?");
                serial_puts("\n");
                lua_pop(L, 1);
            }
            for (volatile int d = 0; d < 800000; d++) {} /* ~8ms: cap redraw rate, kills tearing */
        }
    }

    lua_close(L);
    serial_puts("[lua] closed\n");
}
