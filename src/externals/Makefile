extcfiles := $(wildcard src/externals/*.c)

extobjs := $(addsuffix .o,$(basename $(extcfiles)))
allobjs := $(allobjs) $(extobjs)
alllibs := $(alllibs) src/externals/libthrowtheswitch.a

src/externals/libthrowtheswitch.a: $(extobjs)
	$(AR) rcs $@ $^
