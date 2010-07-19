# Servers Makefile
#

MAKE	=	make
SUBDIRS	=	

# Cibles

all:	sub

sub:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Entering $$dir"; \
	$(MAKE); \
	cd ..; \
	done

clean:
	@for dir in $(SUBDIRS) ; do \
	cd $$dir; \
	echo "Cleaning $$dir"; \
	$(MAKE) clean ; \
	cd ..; \
	done