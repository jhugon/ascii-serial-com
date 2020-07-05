.SUFFIXES:

ifndef build_type
build_type := debug
export build_type
endif

ifndef CC
CC := gcc
export CC
endif

OBJDIR := build_$(build_type)

MAKETARGET = $(MAKE) --no-print-directory -C $@ -f $(CURDIR)/Makefile \
                 SRCDIR=$(CURDIR) $(MAKECMDGOALS)

.PHONY: $(OBJDIR)
$(OBJDIR):
	+@[ -d $@ ] || mkdir -p $@
	+@$(MAKETARGET)

Makefile : ;
%.mk :: ;

% :: $(OBJDIR) ; :

.PHONY: clean
clean:
	rm -rf $(OBJDIR)
