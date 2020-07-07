testcfiles := $(wildcard src/test/*.c)
testobjs := $(addsuffix .o,$(basename $(testcfiles)))
testexes := $(basename $(testcfiles))

ifeq ($(platform),native)
  allobjs := $(allobjs) $(testobjs)
  allexes := $(allexes) $(testexes)
endif

$(testexes): %: %.o src/libasciiserialcom.a src/externals/libthrowtheswitch.a
	$(CC) $(CFLAGS) -o $@ $^
