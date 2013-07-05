# RhinOS Makefile
#

MAKE	:=	make
SUBDIRS	:=	kern lib srv
RM	:=	rm -f
KERN	:=	kern/kern
USER	:=	srv/user
LD_KERN	:=	ld -s -T link.ld
LD_USER	:=	ld -s -T link_user.ld
CFLAGS	:=	-Iinclude -Iinclude/arch/x86


# Objects
OBJ_USER = srv/user.o
OBJ_KERN = kern/khead.o kern/klib_s.o kern/klib_c.o kern/interrupt.o kern/start.o kern/seg.o kern/tables.o kern/pic.o kern/pit.o kern/irq.o kern/exceptions.o kern/physmem.o kern/paging.o kern/virtmem_buddy.o kern/virtmem_slab.o kern/virtmem.o kern/thread.o kern/sched.o kern/syscall.o kern/proc.o kern/main.o
OBJ_IPC  = lib/ipc/ipc.o

all:	user kern

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
