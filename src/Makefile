
LDFLAGS=$(CFLAGS)

ifneq (,$(findstring gcc,$(CC)))
  GCCFLAGS=-Werror -pedantic -pedantic-errors -Wall -Wextra
  GCCFLAGS+=-Wdouble-promotion -Wformat=2 -Wformat-signedness -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wuninitialized -Wtrampolines -Wfloat-equal -Wshadow -Wundef -Wbad-function-cast
  CFLAGS=$(GCCFLAGS) -std=c11 #c18
  CXXFLAGS=$(GCCFLAGS) -std=c++14 -Wsuggest-override -Wplacement-new=2
  ifneq (,$(findstring avr,$(CC)))
  else
    CFLAGS+= -Wformat-overflow=2 -Wformat-truncation=2 -Wnull-dereference -Walloc-zero -Wduplicated-branches -Wduplicated-cond
  endif
  ifeq ($(build_type),debug)
    CFLAGS+=-g -Og
    ifneq (,$(findstring avr,$(CC)))
    else
      CFLAGS+=-fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fsanitize-address-use-after-scope -fstack-protector-all #-fsanitize=pointer-compare -fsanitize=pointer-subtract
    endif
  else
    CFLAGS+=-O2 -flto -Wstrict-aliasing -fstrict-aliasing
  endif
  else
endif

all: test

test: test.o

test.o: test.c

cppcheck:
	cppcheck .

.PHONY: clean
clean:
	rm -rf test *.o
