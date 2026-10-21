#ifndef GFX_H
#define GFX_H

struct limine_framebuffer;
struct lua_State;

void gfx_set_framebuffer(struct limine_framebuffer *fb);
void gfx_register(struct lua_State *L);

#endif
