OUTPUT_ARCH("riscv")    /* 说明输出文件的目标架构位RISC-V */
ENTRY(_entry)           /* 指定程序入口点为 _entry */   

SECTIONS
{
	/*
	* 确保 _entry 位于目标地址 0x80000000，
	* qemu's -kernel 跳转到此处。
	*/
	. = 0x80000000;

	/* 
	* 代码段（4kB 对齐）
	*/
	.text : {
		*(.text .text.*)
		. = ALIGN(0x1000);
		_trampoline = .;
		*(trampsec);
		. = ALIGN(0x1000);
		ASSERT(. - _trampoline == 0x1000, "error: trampoline larger than one page");
		PROVIDE(etext = .);
	}
    
	/*
	* 只读数据段（16B 对齐）
	*/
	.rodata : {
		. = ALIGN(16);
		*(.srodata .sradata.*)
		. = ALIGN(16);
		*(.rodata .radata.*)
	}

	/*
	* 数据段（16B 对齐）
	*/
	.data : {
		. = ALIGN(16);
		*(.sdata .sdata.*)
		. = ALIGN(16);
		*(.data .data.*)
	}

	/*
	* 未初始化数据段（16B 对齐）
	*/
	.bss : {
		. = ALIGN(16);
		*(.sbss .sbss.*)
		. = ALIGN(16);
		*(.bss .bss.*)
	}52
    	
	/*
	* 动态链接信息段（16B 对齐）
	*/
	.got : {
		. = ALIGN(16);
		*(.got .got.*)
	}

	PROVIDE(end = .);
}
