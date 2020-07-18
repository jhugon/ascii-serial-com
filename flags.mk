
#$(info CC is ${CC})
#$(info build_type is ${build_type})

ifneq (,$(findstring gcc,$(CC)))
  GCCFLAGS=-Werror -Wall -Wextra
  GCCFLAGS+=-Wdouble-promotion -Wformat=2 -Wformat-signedness -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wuninitialized -Wtrampolines -Wfloat-equal -Wshadow -Wundef -Wbad-function-cast
  CFLAGS=$(GCCFLAGS)
  CXXFLAGS=$(GCCFLAGS) -std=gnu++17 -Wsuggest-override -Wplacement-new=2
  ifneq (,$(findstring avr,$(CC)))
  else
    CFLAGS+= -std=gnu18 -Wformat-overflow=2 -Wformat-truncation=2 -Wnull-dereference -Walloc-zero -Wduplicated-branches -Wduplicated-cond
  endif
  ifeq ($(build_type),debug)
    CFLAGS+=-g -Og
    ifneq (,$(findstring avr,$(CC)))
    else
      CFLAGS+=-fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fsanitize-address-use-after-scope -fstack-protector-all #-fsanitize=pointer-compare -fsanitize=pointer-subtract
	  ifdef coverage
		CFLAGS+=--coverage
	  endif
    endif
  else
    CFLAGS+=-O2 -flto -Wstrict-aliasing -fstrict-aliasing
  endif
  else
endif

ifneq (,$(findstring clang,$(CC)))
  CLANGFLAGS=-Werror -Wall -Wextra
  CLANGFLAGS+=-Wdouble-promotion -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wuninitialized -Wfloat-equal -Wshadow -Wundef -Wbad-function-cast
  CFLAGS=$(CLANGFLAGS) -std=gnu18
  CXXFLAGS=$(CLANGFLAGS) -std=gnu++14 -Wsuggest-override -Wplacement-new=2
  ifeq ($(build_type),debug)
    CFLAGS+=-g -Og -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fsanitize-address-use-after-scope -fstack-protector-all #-fsanitize=pointer-compare -fsanitize=pointer-subtract
	ifdef coverage
	  CFLAGS+=--coverage
	endif
  else
    CFLAGS+=-O2
  endif
endif

LDFLAGS=$(CFLAGS)
CFLAGS+=-Isrc/
