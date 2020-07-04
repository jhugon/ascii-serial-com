cfiles=$(wildcard *.c)

objs=$(addsuffix .o,$(basename $(cfiles)))

libthrowtheswitch.a: $(objs)
	$(AR) rcs $@ $^

$(objs): %.o: %.c

.PHONY: clean
clean:
	rm -rf libthrowtheswitch.a *.o
