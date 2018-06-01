TARGET=file0
CC=clang
CC?=clang
CC_FLAGS=
CC_EXTRA?=
CC_FLAGS+=$(CC_EXTRA)

SRCS=$(wildcard *.c)
WASM=$(patsubst %.c, %.wasm, $(wildcard *.c))

.DEFAULT:all

.PHONY:all clean help

all:$(TARGET) wasmforce

depend:.depend

.depend:$(SRCS)
	rm -rf .depend
	$(CC) -MM $(CC_FLAGS) $^ > ./.depend

-include ./.depend

%.wasm:%.c
	../../../../extra-tools/llvm-wasm $<

wasmforce:$(WASM)
	@echo forcing generation of wasm and wast

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@ 

$(TARGET): $(TARGET).o
	$(CC) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.s *.wasm *.wast *.bc *.o *~ $(TARGET)
	rm .depend

help:
	@echo "all is the default target"
	@echo "there is delete."
	@echo "there is clean."
