ENTRY(ResetISR)
MEMORY
{
    FLASH (RX) : ORIGIN = (0x2000 +0x100), LENGTH = 0x19000
    SRAM (RWX) : ORIGIN = 0x20000000, LENGTH = 0x00005000
    GPRAM (RWX) : ORIGIN = 0x11000000, LENGTH = 0x00002000
}
_estack = ORIGIN(SRAM) + LENGTH(SRAM);
_Min_Heap_Size = 0;
_Min_Stack_Size = 0x100;
SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH
    .data :
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM AT > FLASH
    .ARM.exidx :
    {
        *(.ARM.exidx*)
    } > FLASH
    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM
    ._user_heap_stack :
    {
      . = ALIGN(4);
      . = . + _Min_Heap_Size;
      . = . + _Min_Stack_Size;
      . = ALIGN(4);
    } > SRAM
    .gpram :
    {
    } > GPRAM
}
