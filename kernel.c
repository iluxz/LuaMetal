#include <stdint.h>
#include "limine.h"
#include "serial.h"

LIMINE_BASE_REVISION(1)

static volatile struct limine_framebuffer_request fb_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST
};

void lua_run(void);

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
    serial_puts("\nLuaMetal milestone 2: limine booted the kernel\n");

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

    lua_run();

    serial_puts("halting.\n");
    for (;;) {
        __asm__ volatile("hlt");
    }
}



