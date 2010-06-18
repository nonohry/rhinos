# RhinOS Makefile
#

MAKE	=	make
SUBDIRS	=	boot kern
DD	=	dd
RM	=	rm -f
IMG	=	disk.img
SIZE	=	2880 # secteurs de 512
BOOT0	=	boot/boot0
BOOT1	= 	boot/boot1
KERN	=	kern/kern

sub:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Entering $$dir"; \
	$(MAKE); \
	cd ..; \
	done

img:	sub
	cat /dev/zero | $(DD) of=$(IMG) count=$(SIZE) bs=512 conv=notrunc
	cat $(BOOT0) | $(DD) of=$(IMG) bs=512 conv=notrunc
	cat $(BOOT1) | $(DD) of=$(IMG) seek=1 bs=512 conv=notrunc
	cat $(KERN)  | $(DD) of=$(IMG) seek=3 bs=512 conv=notrunc

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