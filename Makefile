# RhinOS Makefile
#

MAKE	=	make
SUBDIRS	=	boot kern lib
DD	=	dd
RM	=	rm -f
IMG	=	disk.img
SIZE	=	10
BOOT0	=	boot/boot0
BOOT1	= 	boot/boot1
KERN	=	kern/kern
LD	=	ld -s -T link.ld
CFLAGS	=	-Iinclude


# Objets

OBJ_KERN = kern/khead.o kern/klib.o kern/interrupt.o kern/start.o kern/seg.o kern/tables.o kern/pic.o kern/pit.o kern/irq.o kern/exceptions.o kern/physmem.o kern/paging.o kern/virtmem_buddy.o kern/virtmem_slab.o kern/virtmem.o kern/context_cpu.o kern/thread.o kern/sched.o kern/syscall.o kern/main.o
OBJ_IPC  = lib/ipc/ipc.o

# Offset en secteurs des differents modules

BOOTSEEK	=	1
KERNSEEK	=	3



sub:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Entering $$dir"; \
	$(MAKE) depend; \
	$(MAKE) all; \
	cd ..; \
	done

kern: 	sub
	$(LD) $(CFLAGS) -o $(KERN) $(OBJ_KERN) $(OBJ_IPC)	

hd:	sub kern
	echo yes | bximage -q -hd -mode=flat -size=$(SIZE) $(IMG) 
	cat $(BOOT0) | $(DD) of=$(IMG) bs=512 conv=notrunc
	cat $(BOOT1) | $(DD) of=$(IMG) bs=512 seek=$(BOOTSEEK) conv=notrunc
	cat $(KERN)  | $(DD) of=$(IMG) bs=512 seek=$(KERNSEEK) conv=notrunc

fd:	sub kern
	echo yes | bximage -q -fd  -size=1.44 $(IMG) 
	cat $(BOOT0) | $(DD) of=$(IMG) bs=512 conv=notrunc
	cat $(BOOT1) | $(DD) of=$(IMG) bs=512 seek=$(BOOTSEEK) conv=notrunc
	cat $(KERN)  | $(DD) of=$(IMG) bs=512 seek=$(KERNSEEK) conv=notrunc

clean:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Cleaning $$dir"; \
	$(MAKE) clean ; \
	cd ..; \
	done
	@echo "Cleaning current dir";
	$(RM) $(IMG)
	$(RM) *~
