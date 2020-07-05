
srcobjs := src/test.o src/functest.o src/circular_buffer.o
allobjs := $(allojbs) $(srcobjs)
allexes := $(allexes) src/test

all: src/test

src/test: $(srcobjs) src/externals/libthrowtheswitch.a

cppcheck:
	cppcheck .

include src/externals/Makefile
