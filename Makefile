# RhinOS Makefile
#

MAKE	=	make
SUBDIRS	=	boot kern 
DD	=	dd
RM	=	rm -f
IMG	=	disk.img
SIZE	=	10
BOOT0	=	boot/boot0
BOOT1	= 	boot/boot1
KERN	=	kern/kern

# Offset en secteurs des differents modules

BOOTSEEK	=	1
KERNSEEK	=	3

sub:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Entering $$dir"; \
	$(MAKE) depend; \
	$(MAKE); \
	cd ..; \
	done

img:	sub
	cat /dev/zero | $(DD) of=$(IMG) count=$(SIZE) bs=512 conv=notrunc
	cat $(BOOT0) | $(DD) of=$(IMG) bs=512 conv=notrunc
	cat $(BOOT1) | $(DD) of=$(IMG) seek=$(BOOTSEEK) bs=512 conv=notrunc
	cat $(KERN)  | $(DD) of=$(IMG) seek=$(KERNSEEK) bs=512 conv=notrunc

hd:	sub
	echo yes | bximage -q -hd -mode=flat -size=$(SIZE) $(IMG) 
	cat $(BOOT0) | $(DD) of=$(IMG) bs=512 conv=notrunc
	cat $(BOOT1) | $(DD) of=$(IMG) bs=512 seek=$(BOOTSEEK) conv=notrunc
	cat $(KERN)  | $(DD) of=$(IMG) bs=512 seek=$(KERNSEEK) conv=notrunc

fd:	sub
	echo yes | bximage -q -fd  -size=1.44 img 
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
