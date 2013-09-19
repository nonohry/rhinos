# RhinOS Makefile
#

ARCH	?=	x86

MAKE	:=	make
SUBDIRS	:=	kern kern/arch/$(ARCH) srv #lib
RM	:=	rm -f
KERN	:=	kern/kern
USER_SEND	:=	srv/user_send
USER_RECV	:=	srv/user_recv
LD_KERN	:=	ld -s -T link.ld
LD_USER	:=	ld -s -T link_user.ld
CFLAGS	:=	-Iinclude -Iinclude/arch/x86


# Objects
OBJ_USER_SEND = srv/user_send.o 
OBJ_USER_RECV = srv/user_recv.o
OBJ_KERN = kern/arch/$(ARCH)/krt.o  kern/arch/$(ARCH)/serial.o  kern/arch/$(ARCH)/x86_lib.o kern/arch/$(ARCH)/vm_segment.o kern/arch/$(ARCH)/vm_paging.o kern/arch/$(ARCH)/setup.o kern/arch/$(ARCH)/e820.o kern/arch/$(ARCH)/context.o kern/arch/$(ARCH)/int.o kern/arch/$(ARCH)/pic.o kern/arch/$(ARCH)/exceptions.o  kern/arch/$(ARCH)/pit.o kern/arch/$(ARCH)/interrupt.o kern/main.o kern/pager0.o kern/vm_pool.o kern/vm_slab.o kern/thread.o kern/proc.o kern/sched.o kern/syscall.o kern/irq.o kern/clock.o
OBJ_IPC  = lib/ipc/ipc.o

all:	kern user_send user_recv

sub:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Entering $$dir"; \
	$(MAKE) depend; \
	$(MAKE) all ARCH=$(ARCH); \
	cd -; \
	done

kern: 	sub
	$(LD_KERN) $(CFLAGS) -o $(KERN) $(OBJ_KERN) $(OBJ_IPC)	

user_send:	sub
	$(LD_USER) $(CFLAGS) -o $(USER_SEND) $(OBJ_USER_SEND) $(OBJ_IPC)	

user_recv:	sub
	$(LD_USER) $(CFLAGS) -o $(USER_RECV) $(OBJ_USER_RECV) $(OBJ_IPC)

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
