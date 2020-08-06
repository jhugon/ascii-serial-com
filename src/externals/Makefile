extcfiles := src/externals/CException.c
ifeq ($(platform),native)
  extcfiles := $(extcfiles) src/externals/cmock.c src/externals/unity.c
endif

extobjs := $(addsuffix .o,$(basename $(extcfiles)))
allobjs := $(allobjs) $(extobjs)
alllibs := $(alllibs) src/externals/libthrowtheswitch.a

src/externals/libthrowtheswitch.a: $(extobjs)
	$(AR) rcs $@ $^
