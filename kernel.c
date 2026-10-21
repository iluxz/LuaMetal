#include <stdint.h>
#include "limine.h"
#include "serial.h"
#include "gfx.h"

LIMINE_BASE_REVISION(1)

#define MAX_MODS 16

static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST
};

static volatile struct limine_module_request mod_req = {
    .id = LIMINE_MODULE_REQUEST
};

void lua_run(const char **bufs, unsigned long *lens, int n);
void input_init(void);

void _start(void) {
    unsigned long cr0, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1UL<<2);
    cr0 |= (1UL<<1);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1UL<<9) | (1UL<<10);
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
    serial_init();
    serial_puts("\nLuaMetal milestone 4: limine booted the kernel\n");

    /* set up the framebuffer + hand it to the gfx module */
    if (fb_req.response == 0 || fb_req.response->framebuffer_count == 0) {
        serial_puts("no framebuffer\n");
    } else {
        struct limine_framebuffer *fb = fb_req.response->framebuffers[0];
        gfx_set_framebuffer(fb);
        serial_puts("framebuffer online\n");
    }

    /* collect every boot module (our .lua files) in order */
    const char *bufs[MAX_MODS];
    unsigned long lens[MAX_MODS];
    int n = 0;
    if (mod_req.response) {
        for (uint64_t i = 0; i < mod_req.response->module_count && n < MAX_MODS; i++) {
            struct limine_file *m = mod_req.response->modules[i];
            bufs[n] = (const char *)m->address;
            lens[n] = (unsigned long)m->size;
            serial_puts("[init] module: ");
            serial_puts(m->path ? m->path : "?");
            serial_puts("\n");
            n++;
        }
    }
    if (n == 0) serial_puts("[init] no modules found\n");

    input_init();
    serial_puts("ps/2 input online\n");

    lua_run(bufs, lens, n);

    serial_puts("halting.\n");
    for (;;) {
        __asm__ volatile("hlt");
    }
}
