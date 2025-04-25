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
		*(.text)
		. = ALIGN(0x1000);
	}
    
	/*
	* 只读数据段（16B 对齐）
	*/
	.rodata : {
		*(.rodata)
		. = ALIGN(16);
	}

	/*
	* 数据段（16B 对齐）
	*/
	.data : {
		*(.data)
		. = ALIGN(16);
	}

	/*
	* 未初始化数据段（16B 对齐）
	*/
	.bss : {
		*(.bss)
	. = ALIGN(16);
	}
    	
	/*
	* 动态链接信息段（16B 对齐）
	*/
	.got : {
		*(.got .got.*)
		. = ALIGN(16);
	}

	/*
	* 之后为栈空间开辟地址* 
	*/
	PROVIDE(stack0 = .);
}
