# 此 Makfile 用于编译和运行QEMU模拟的RISC-V64位裸机环境下的程序
K = kernel
U = user

OBJS = \
	$(K)/entry.o \
	$(K)/start.o \
	$(K)/kernelvec.o \
	$(K)/trap.o \
	$(K)/main.o

LINK = $(K)/kernel.ld

QEMU = qemu-system-riscv64

TOOLPREFIX = riscv64-linux-gnu-
CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)as
LD = $(TOOLPREFIX)ld
OBJDUMP = $(TOOLPREFIX)objdump

GDB = riscv64-unknown-elf-gdb

# 基础配置
QEMUOPTS = -machine virt -cpu rv64 -bios none -m 128M -smp 1 -nographic

# 存储设备配置
QEMUOPTS += 

# 网络配置
QEUMOPTS +=

# 定义编译器选项 Compiler Flags，启用所有警告并视为错误
# 启用编译器优化但是不能省略桢指针，生成调试信息
CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb -gdwarf-2
# 自动生成依赖文件
CFLAGS += -MD
# 支持64位地址
CFLAGS += -mcmodel=medany
# 将未初始化全局变量设为强符号，不依赖标准库
CFLAGS += -fno-common -nostdlib
# 禁用内建函数
CFLAGS += -fno-builtin-strncpy -fno-builtin-strncmp -fno-builtin-strlen -fno-builtin-memset
CFLAGS += -fno-builtin-memmove -fno-builtin-memcmp -fno-builtin-log -fno-builtin-bzero
CFLAGS += -fno-builtin-strchr -fno-builtin-exit -fno-builtin-malloc -fno-builtin-putc
CFLAGS += -fno-builtin-free
CFLAGS += -fno-builtin-memcpy -Wno-main
CFLAGS += -fno-builtin-printf -fno-builtin-fprintf -fno-builtin-vprintf
# 将该文件加入搜索头文件
CFLAGS += -I.

# 定义汇编器选项
ASFLAGS = 

# 定义链接器选项
LDFLAGS =

%/%.o: %/%.c
	$(CC) $(CFLAGS) -g -c -o $@ $<

%/%.o: %/%.S
	$(AS) $(ASFLAGS) -g -o $@ $<

$(K)/kernel: $(OBJS)
	$(LD) $(LDFLAGS) -T $(LINK) -o $@ $^
	$(OBJDUMP) -S $@ > $(K)/kernel.asm

qemu: $(K)/kernel 
	$(QEMU) $(QEMUOPTS) -kernel $(K)/kernel

qemu-dtb:
	$(QEMU) $(QEMUOPTS) -machine dumpdtb=vm.dtb
	dtc -I dtb -O dts vm.dtb -o vm.dts

.gdbinit: 
	@echo "target remote localhost:1234" > $@
	@echo "file $(K)/kernel" >> $@
	@echo "layout asm" >> $@

qemu-gdb: $(K)/kernel
	$(QEMU) $(QEMUOPTS) -kernel $(K)/kernel -S -s

gdb: .gdbinit
	$(GDB) -x $<

clean:
	rm -f $(K)kernel.asm $(K)/kernel */*.o .gdbinit vm.*

