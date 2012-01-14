# RhinOS Lib Makefile
#

MAKE	=	make
SUBDIRS	=	ipc
RM	=	rm -f


all:	sub

sub:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Entering $$dir"; \
	$(MAKE) depend; \
	$(MAKE) all; \
	cd ..; \
	done

depend:
	@echo "Top Level Makefile: no depend";

clean:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Cleaning $$dir"; \
	$(MAKE) clean ; \
	cd ..; \
	done

