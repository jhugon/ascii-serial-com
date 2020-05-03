CC=gcc

CBASEFLAGS=-Werror -pedantic -pedantic-errors -Wall -Wextra

CBASEFLAGS+=-Wdouble-promotion -Wformat=2 -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2 -Wnull-dereference -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wuninitialized -Walloc-zero -Wduplicated-branches -Wduplicated-cond -Wtrampolines -Wfloat-equal -Wshadow -Wundef -Wbad-function-cast

CXXFLAGS=$(CBASEFLAGS) -std=c++14 -Wsuggest-override -Wplacement-new=2
CFLAGS=$(CBASEFLAGS) -std=c11 #c18

CFLAGS_DEBUG=-g -Og -fsanitize=address -fsanitize=leak -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fsanitize-address-use-after-scope -fstack-protector-all #-fsanitize=pointer-compare -fsanitize=pointer-subtract

CFLAGS_OPT=-O2 -flto -Wstrict-aliasing -fstrict-aliasing

all: test_debug test_opt cppcheck

test_debug: test.c
	$(CC) $(CFLAGS) $(CFLAGS_DEBUG) test.c -o test_debug

test_opt: test.c
	$(CC) $(CFLAGS) $(CFLAGS_OPT) test.c -o test_opt

test_opt_clang: test.c
	clang test.c -o test_opt_clang

cppcheck:
	cppcheck .

clean:
	rm test_opt test_debug
