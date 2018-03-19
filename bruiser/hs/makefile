TARGET=bruiserhs
CC=clang
CC?=clang
CC_FLAGS=
CC_EXTRA?=
CC_FLAGS+=$(CC_EXTRA)

SRCS=$(wildcard *.c)

.DEFAULT:all clean

.PHONY:all clean help

all:$(TARGET)

depend:.depend

.depend:$(SRCS)
	rm -rf .depend
	$(CC) -MM $(CC_FLAGS) $^ > ./.depend

-include ./.depend

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@ 

$(TARGET): $(TARGET).o
	$(CC) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(TARGET)
	rm .depend

help:
	@echo "all is the default target"
	@echo "there is delete."
	@echo "there is clean."
