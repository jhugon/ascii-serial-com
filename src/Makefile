
LDFLAGS=$(CFLAGS)

ifneq (,$(findstring gcc,$(CC)))
  GCCFLAGS=-Werror -pedantic -pedantic-errors -Wall -Wextra
  GCCFLAGS+=-Wdouble-promotion -Wformat=2 -Wformat-signedness -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wuninitialized -Wtrampolines -Wfloat-equal -Wshadow -Wundef -Wbad-function-cast
  CFLAGS=$(GCCFLAGS)
  CXXFLAGS=$(GCCFLAGS) -std=c++17 -Wsuggest-override -Wplacement-new=2
  ifneq (,$(findstring avr,$(CC)))
  else
    CFLAGS+= -std=c18 -Wformat-overflow=2 -Wformat-truncation=2 -Wnull-dereference -Walloc-zero -Wduplicated-branches -Wduplicated-cond
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

ifneq (,$(findstring clang,$(CC)))
  CLANGFLAGS=-Werror -pedantic -pedantic-errors -Wall -Wextra
  CLANGFLAGS+=-Wdouble-promotion -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wuninitialized -Wfloat-equal -Wshadow -Wundef -Wbad-function-cast
  CFLAGS=$(CLANGFLAGS) -std=c18
  CXXFLAGS=$(CLANGFLAGS) -std=c++14 -Wsuggest-override -Wplacement-new=2
  ifeq ($(build_type),debug)
    CFLAGS+=-g -Og -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fsanitize-address-use-after-scope -fstack-protector-all #-fsanitize=pointer-compare -fsanitize=pointer-subtract
  else
    CFLAGS+=-O2
  endif
endif

srcobjs := src/test.o src/functest.o src/circular_buffer.o
allobjs := $(allojbs) $(srcobjs)
allexes := $(allexes) src/test

all: src/test

src/test: $(srcobjs) src/externals/libthrowtheswitch.a

cppcheck:
	cppcheck .

include src/externals/Makefile
