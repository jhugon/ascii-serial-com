
LDFLAGS=$(CFLAGS)

ifeq ($(CC),gcc)
  GCCFLAGS=-Werror -pedantic -pedantic-errors -Wall -Wextra
  GCCFLAGS+=-Wdouble-promotion -Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2 -Wnull-dereference -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wuninitialized -Walloc-zero -Wduplicated-branches -Wduplicated-cond -Wtrampolines -Wfloat-equal -Wshadow -Wundef -Wbad-function-cast
  CFLAGS=$(GCCFLAGS) -std=c11 #c18
  CXXFLAGS=$(GCCFLAGS) -std=c++14 -Wsuggest-override -Wplacement-new=2
  ifeq ($(build_type),debug)
    CFLAGS+=-g -Og -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fsanitize-address-use-after-scope -fstack-protector-all #-fsanitize=pointer-compare -fsanitize=pointer-subtract
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
	rm test_opt test_debug
