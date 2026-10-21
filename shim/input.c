#include "input.h"
#include "gfx.h"

static inline void outb(unsigned short p, unsigned char v) {
    __asm__ volatile("outb %0, %1" : : "a"(v), "Nd"(p));
}
static inline unsigned char inb(unsigned short p) {
    unsigned char v;
    __asm__ volatile("inb %1, %0" : "=a"(v) : "Nd"(p));
    return v;
}

static void wait_in(void) { int i = 0; while ((inb(0x64) & 2) && i++ < 100000) {} }
static void wait_out(void) { int i = 0; while (!(inb(0x64) & 1) && i++ < 100000) {} }

/* mouse packet state */
static int mphase;
static unsigned char mp[3];
static int m_x, m_y, m_btn, m_dx, m_dy;

/* keyboard scancode ring buffer */
static unsigned char kq[64];
static int kr, kw;

static void mouse_byte(unsigned char b) {
    if (mphase == 0) { if (b & 8) { mp[0] = b; mphase = 1; } return; }
    if (mphase == 1) { mp[1] = b; mphase = 2; return; }
    mp[2] = b;
    mphase = 0;
    m_btn = mp[0] & 7;
    m_dx += (signed char)mp[1];
    m_dy -= (signed char)mp[2];
}

static void kbd_byte(unsigned char b) {
    int n = (kw + 1) & 63;
    if (n != kr) { kq[kw] = b; kw = n; }
}

void input_init(void) {
    /* read controller config byte */
    wait_in(); outb(0x64, 0x20); wait_out();
    unsigned char cfg = inb(0x60);
    cfg |= 0x02;                 /* mouse interrupt (we poll, but keep it enabled) */
    cfg &= (unsigned char)~0x30; /* don't disable either clock */
    wait_in(); outb(0x64, 0x60); wait_in(); outb(0x60, cfg);
    /* enable auxiliary device */
    wait_in(); outb(0x64, 0xA8);
    /* tell mouse to start reporting */
    wait_in(); outb(0x64, 0xD4); wait_in(); outb(0x60, 0xF4);
    wait_out(); inb(0x60);       /* ack */
    mphase = 0;
    m_x = gfx_width() / 2;
    m_y = gfx_height() / 2;
}

void input_poll(void) {
    int guard = 0;
    while ((inb(0x64) & 1) && guard++ < 64) {
        unsigned char st = inb(0x64);
        unsigned char b = inb(0x60);
        if (st & 0x20) mouse_byte(b); else kbd_byte(b);
    }
    int w = gfx_width(), h = gfx_height();
    m_x += m_dx; m_y += m_dy;
    m_dx = 0; m_dy = 0;
    if (m_x < 0) m_x = 0;
    if (m_y < 0) m_y = 0;
    if (m_x > w - 1) m_x = w - 1;
    if (m_y > h - 1) m_y = h - 1;
}

int input_x(void) { return m_x; }
int input_y(void) { return m_y; }
int input_btn(void) { return m_btn; }

int input_scancode(void) {
    if (kr == kw) return -1;
    unsigned char b = kq[kr];
    kr = (kr + 1) & 63;
    return b;
}
