MEMORY {
	rom  (rx) : ORIGIN = 0x00000000, LENGTH = 40K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 8K
}

SECTIONS {
	.text : {
		_text = .;
		KEEP(*(.isr_vector))
		*(.text*)
		*(.rodata*)
		_etext = .;
	} > rom

	.data : AT(ADDR(.text) + SIZEOF(.text)) {
		_data = .;
		*(vtable)
		*(.data*)
		_edata = .;
	} > ram

	.bss : {
		_bss = .;
		*(.bss*)
		*(COMMON)
		_ebss = .;
	} > ram
}
