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

GDB = gdb-multiarch

# 基础配置
QEMUOPTS = -machine virt -cpu rv64 -bios none -m 128M -smp 1 -nographic

# 存储设备配置
QEMUOPTS += 

# 网络配置
QEUMOPTS +=

# 定义编译器选项 Compiler Flags
# 启用所有警告并视为错误
# 启用编译器优化但是不能省略桢指针
# 生成调试信息
CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb

# 禁止依赖宿主环境与标准库
CFLAGS += -ffreestanding -nostdlib

# 支持64位地址
CFLAGS += -mcmodel=medany

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

