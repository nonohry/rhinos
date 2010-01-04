# RhinOS Makefile
#

MAKE	=	make
SUBDIRS	=	boot kern
DD	=	dd
RM	=	rm -f
IMG	=	disk.img
SIZE	=	2880 # secteurs de 512
OBJ	=	boot/boot0 boot/boot1 kern/kern


sub:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Entering $$dir"; \
	$(MAKE); \
	cd ..; \
	done

img:	sub
	cat $(OBJ) /dev/zero | $(DD) of=$(IMG) count=$(SIZE) bs=512 conv=notrunc

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