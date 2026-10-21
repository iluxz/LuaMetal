#include <lua.h>
#include <lauxlib.h>
#include "serial.h"
#include "limine.h"
#include "gfx.h"

static uint8_t  *g_fb;
static uint64_t  g_width, g_height, g_pitch;
static uint16_t  g_bpp;
static uint8_t   g_rs, g_rsh;
static uint8_t   g_gs, g_gh;
static uint8_t   g_bs, g_bsh;

void gfx_set_framebuffer(struct limine_framebuffer *fb) {
    g_fb    = (uint8_t *)fb->address;
    g_width = fb->width;
    g_height= fb->height;
    g_pitch = fb->pitch;
    g_bpp   = fb->bpp;
    g_rs = fb->red_mask_size;   g_rsh = fb->red_mask_shift;
    g_gs = fb->green_mask_size; g_gh  = fb->green_mask_shift;
    g_bs = fb->blue_mask_size;  g_bsh = fb->blue_mask_shift;
}

/* scale an 8-bit channel (0-255) into an n-bit mask */
static inline uint32_t scale(uint32_t v, uint8_t n) {
    if (n == 0) return 0;
    if (n >= 8) return v;
    return (v * ((1u << n) - 1)) / 255;
}

/* convert 0xRRGGBB into the framebuffer's native pixel format */
static inline uint32_t conv(uint32_t c) {
    uint32_t r = scale((c >> 16) & 0xFF, g_rs);
    uint32_t g = scale((c >>  8) & 0xFF, g_gs);
    uint32_t b = scale( c        & 0xFF, g_bs);
    return (r << g_rsh) | (g << g_gh) | (b << g_bsh);
}

static inline void put(uint32_t px, int x, int y) {
    if (x < 0 || y < 0 || x >= (int)g_width || y >= (int)g_height) return;
    if (g_bpp == 32) {
        uint32_t *row = (uint32_t *)(g_fb + (uint64_t)y * g_pitch);
        row[x] = px;
    } else if (g_bpp == 24) {
        uint8_t *p = g_fb + (uint64_t)y * g_pitch + (uint64_t)x * 3;
        p[0] = px & 0xFF; p[1] = (px >> 8) & 0xFF; p[2] = (px >> 16) & 0xFF;
    }
}

static int l_width(lua_State *L)  { lua_pushinteger(L, (lua_Integer)g_width);  return 1; }
static int l_height(lua_State *L) { lua_pushinteger(L, (lua_Integer)g_height); return 1; }

int gfx_width(void)  { return (int)g_width; }
int gfx_height(void) { return (int)g_height; }

static int l_pixel(lua_State *L) {
    int x = (int)luaL_checkinteger(L, 1);
    int y = (int)luaL_checkinteger(L, 2);
    uint32_t c = (uint32_t)luaL_checkinteger(L, 3);
    put(conv(c), x, y);
    return 0;
}

static int l_rect(lua_State *L) {
    int x = (int)luaL_checkinteger(L, 1);
    int y = (int)luaL_checkinteger(L, 2);
    int w = (int)luaL_checkinteger(L, 3);
    int h = (int)luaL_checkinteger(L, 4);
    uint32_t c = (uint32_t)luaL_checkinteger(L, 5);
    uint32_t px = conv(c);
    for (int j = y; j < y + h; j++)
        for (int i = x; i < x + w; i++)
            put(px, i, j);
    return 0;
}

static int l_clear(lua_State *L) {
    uint32_t c = (uint32_t)luaL_optinteger(L, 1, 0x000000);
    uint32_t px = conv(c);
    for (uint64_t j = 0; j < g_height; j++) {
        if (g_bpp == 32) {
            uint32_t *row = (uint32_t *)(g_fb + j * g_pitch);
            for (uint64_t i = 0; i < g_width; i++) row[i] = px;
        } else {
            for (uint64_t i = 0; i < g_width; i++) put(px, (int)i, (int)j);
        }
    }
    return 0;
}

void gfx_register(lua_State *L) {
    static const luaL_Reg fns[] = {
        {"width",  l_width},
        {"height", l_height},
        {"pixel",  l_pixel},
        {"rect",   l_rect},
        {"clear",  l_clear},
        {NULL, NULL}
    };
    luaL_newlib(L, fns);
    lua_setglobal(L, "gfx");
}
