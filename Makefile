# RhinOS Makefile
#

MAKE	=	make
SUBDIRS	=	boot kern lib srv
DD	=	dd
RM	=	rm -f
IMG	=	disk.img
SIZE	=	10
BOOT0	=	boot/boot0
BOOT1	= 	boot/boot1
KERN	=	kern/kern
USER	=	srv/user
LD_KERN	=	ld -s -T link.ld
LD_USER	=	ld -s -T link_user.ld
CFLAGS	=	-Iinclude


# Objets
OBJ_USR = srv/user.o
OBJ_KERN = kern/khead.o kern/klib_s.o kern/klib_c.o kern/interrupt.o kern/start.o kern/seg.o kern/tables.o kern/pic.o kern/pit.o kern/irq.o kern/exceptions.o kern/physmem.o kern/paging.o kern/virtmem_buddy.o kern/virtmem_slab.o kern/virtmem.o kern/thread.o kern/sched.o kern/syscall.o kern/proc.o kern/main.o
OBJ_IPC  = lib/ipc/ipc.o

# Offset en secteurs des differents modules

BOOTSEEK	=	1
KERNSEEK	=	3
USERSEEK	=	103


sub:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Entering $$dir"; \
	$(MAKE) depend; \
	$(MAKE) all; \
	cd ..; \
	done

kern: 	sub
	$(LD_KERN) $(CFLAGS) -o $(KERN) $(OBJ_KERN) $(OBJ_IPC)	

user:	sub
	$(LD_USER) $(CFLAGS) -o $(USER) $(OBJ_USER) $(OBJ_IPC)	

hd:	sub kern
	@echo yes | bximage -q -hd -mode=flat -size=$(SIZE) $(IMG) 1>/dev/null 
	@cat $(BOOT0) | $(DD) of=$(IMG) bs=512 conv=notrunc 2>/dev/null
	@cat $(BOOT1) | $(DD) of=$(IMG) bs=512 seek=$(BOOTSEEK) conv=notrunc 2>/dev/null
	@cat $(KERN)  | $(DD) of=$(IMG) bs=512 seek=$(KERNSEEK) conv=notrunc 2>/dev/null
	@cat $(USER)  | $(DD) of=$(IMG) bs=512 seek=$(USERSEEK) conv=notrunc 2>/dev/null

fd:	sub kern
	@echo yes | bximage -q -fd  -size=1.44 $(IMG) 1>/dev/null
	@cat $(BOOT0) | $(DD) of=$(IMG) bs=512 conv=notrunc 2>/dev/null
	@cat $(BOOT1) | $(DD) of=$(IMG) bs=512 seek=$(BOOTSEEK) conv=notrunc 2>/dev/null
	@cat $(KERN)  | $(DD) of=$(IMG) bs=512 seek=$(KERNSEEK) conv=notrunc 2>/dev/null
	@cat $(USER)  | $(DD) of=$(IMG) bs=512 seek=$(USERSEEK) conv=notrunc 2>/dev/null


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
