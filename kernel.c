#include <stdint.h>
#include "limine.h"
#include "serial.h"

LIMINE_BASE_REVISION(1)

static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST
};

static volatile struct limine_module_request mod_req = {
    .id = LIMINE_MODULE_REQUEST
};

void lua_run(const char *buf, unsigned long len);

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
    serial_puts("\nLuaMetal milestone 3: limine booted the kernel\n");

    if (fb_req.response == 0 || fb_req.response->framebuffer_count == 0) {
        serial_puts("no framebuffer\n");
    } else {
        struct limine_framebuffer *fb = fb_req.response->framebuffers[0];
        serial_puts("framebuffer online\n");
        uint32_t color = 0x2C2D72;
        uint32_t *pixels = (uint32_t *)fb->address;
        for (uint64_t y = 0; y < fb->height; y++) {
            for (uint64_t x = 0; x < fb->width; x++) {
                pixels[y * (fb->pitch / 4) + x] = color;
            }
        }
    }

    /* find the init.lua boot module */
    const char *init_buf = 0;
    unsigned long init_len = 0;
    if (mod_req.response && mod_req.response->module_count > 0) {
        struct limine_file *found = 0;
        for (uint64_t i = 0; i < mod_req.response->module_count; i++) {
            struct limine_file *m = mod_req.response->modules[i];
            const char *p = m->path;
            /* match trailing "init.lua" */
            int match = p && p[0];
            if (match) {
                const char *q = p;
                while (*q) q++;
                /* q now at NUL; walk back 8 chars for "init.lua" */
                if (q - p >= 8) {
                    const char *c = q - 8;
                    const char *want = "init.lua";
                    int ok = 1;
                    for (int k = 0; k < 8; k++) if (c[k] != want[k]) { ok = 0; break; }
                    match = ok;
                } else match = 0;
            }
            if (match) { found = m; break; }
        }
        if (!found) found = mod_req.response->modules[0];
        if (found) {
            init_buf = (const char *)found->address;
            init_len = (unsigned long)found->size;
            serial_puts("[init] loaded module: ");
            serial_puts(found->path ? found->path : "?");
            serial_puts("\n");
        }
    }
    if (!init_buf) serial_puts("[init] no module found, using fallback\n");

    lua_run(init_buf, init_len);

    serial_puts("halting.\n");
    for (;;) {
        __asm__ volatile("hlt");
    }
}



