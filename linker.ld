ENTRY(_start)

SECTIONS
{
    . = 0xFFFFFFFF80100000;
    .text : ALIGN(4096) { *(.text .text.*) }
    . = ALIGN(4096);
    .rodata : ALIGN(4096) { *(.rodata .rodata.*) }
    . = ALIGN(4096);
    .data : ALIGN(4096) { *(.data .data.*) }
    . = ALIGN(4096);
    .bss : ALIGN(4096) { *(COMMON) *(.bss .bss.*) }
}
