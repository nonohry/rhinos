# Kernel Makefile
#

# Macros

ARCH	?=	x86

AS	:=	nasm -f elf
CC	:=	gcc -Wall -c
RM	:=	rm -f
LD	:=	ld -s -T link.ld
CFLAGS	:=	-I../include -I../include/arch/${ARCH} -Iarch/${ARCH}

# Suffixes rules
.SUFFIXES: .s

.s.o:
	$(AS) $<

# Files
ASM_SRC	=	#khead.s klib_s.s interrupt.s
ASM_OUT	=	${ASM_SRC:.s=.o}
#C_SRC	=	start.c seg.c tables.c pic.c pit.c irq.c exceptions.c physmem.c paging.c virtmem_buddy.c virtmem_slab.c virtmem.c thread.c sched.c syscall.c klib_c.c proc.c main.c 
C_SRC 	=	main.c thread.c proc.c sched.c pager0.c vm_pool.c vm_slab.c syscall.c irq.c clock.c
C_OUT	=	${C_SRC:.c=.o}
OBJ	=	$(ASM_OUT) $(C_OUT)

# Targets

all: $(OBJ)

asm:	$(ASM_OUT)

depend:
	makedepend -- $(CFLAGS) -- $(C_SRC)

clean:
	$(RM) *~
	$(RM) $(ASM_OUT) $(C_OUT) kern

# DO NOT DELETE

main.o: ../include/define.h ../include/arch/x86/types.h arch/x86/arch_io.h
main.o: arch/x86/serial.h arch/x86/x86_lib.h arch/x86/x86_const.h
main.o: arch/x86/context.h arch/x86/arch_hw.h arch/x86/pic.h thread.h
main.o: ../include/llist.h arch/x86/arch_ctx.h proc.h arch/x86/arch_vm.h
main.o: arch/x86/vm_paging.h irq.h arch/x86/interrupt.h boot.h pager0.h
main.o: vm_pool.h vm_slab.h sched.h clock.h
thread.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
thread.o: arch/x86/arch_io.h arch/x86/serial.h arch/x86/x86_lib.h
thread.o: arch/x86/x86_const.h arch/x86/context.h arch/x86/arch_const.h
thread.o: arch/x86/vm_paging.h arch/x86/arch_ctx.h vm_slab.h sched.h thread.h
thread.o: proc.h arch/x86/arch_vm.h
proc.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
proc.o: arch/x86/arch_io.h arch/x86/serial.h arch/x86/x86_lib.h
proc.o: arch/x86/x86_const.h arch/x86/context.h arch/x86/arch_vm.h
proc.o: arch/x86/vm_paging.h vm_pool.h vm_slab.h thread.h arch/x86/arch_ctx.h
proc.o: proc.h
sched.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
sched.o: thread.h arch/x86/arch_ctx.h arch/x86/context.h proc.h
sched.o: arch/x86/arch_vm.h arch/x86/vm_paging.h sched.h
pager0.o: ../include/define.h ../include/arch/x86/types.h
pager0.o: arch/x86/arch_const.h arch/x86/x86_const.h arch/x86/context.h
pager0.o: arch/x86/vm_paging.h boot.h pager0.h arch/x86/arch_io.h
pager0.o: arch/x86/serial.h arch/x86/x86_lib.h
vm_pool.o: ../include/define.h ../include/arch/x86/types.h
vm_pool.o: arch/x86/arch_const.h arch/x86/x86_const.h arch/x86/context.h
vm_pool.o: arch/x86/vm_paging.h boot.h vm_pool.h
vm_slab.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
vm_slab.o: arch/x86/arch_const.h arch/x86/x86_const.h arch/x86/context.h
vm_slab.o: arch/x86/vm_paging.h vm_pool.h vm_slab.h arch/x86/arch_io.h
vm_slab.o: arch/x86/serial.h arch/x86/x86_lib.h
syscall.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
syscall.o: ../include/ipc.h arch/x86/arch_const.h arch/x86/x86_const.h
syscall.o: arch/x86/context.h arch/x86/vm_paging.h arch/x86/arch_ctx.h proc.h
syscall.o: arch/x86/arch_vm.h thread.h sched.h syscall.h
irq.o: ../include/define.h ../include/arch/x86/types.h ../include/llist.h
irq.o: arch/x86/arch_hw.h arch/x86/pic.h arch/x86/x86_lib.h
irq.o: arch/x86/x86_const.h arch/x86/context.h irq.h arch/x86/interrupt.h
clock.o: ../include/define.h ../include/arch/x86/types.h arch/x86/arch_io.h
clock.o: arch/x86/serial.h arch/x86/x86_lib.h arch/x86/x86_const.h
clock.o: arch/x86/context.h arch/x86/arch_vm.h arch/x86/vm_paging.h irq.h
clock.o: arch/x86/interrupt.h thread.h ../include/llist.h arch/x86/arch_ctx.h
clock.o: proc.h sched.h clock.h
