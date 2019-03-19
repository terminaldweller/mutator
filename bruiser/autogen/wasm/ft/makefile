TARGET=autowasm
SHELL=bash
SHELL?=bash
CC=clang
CC?=clang
CC_FLAGS= -fPIC
CC_EXTRA?=
CTAGS_I_PATH?=./
LD_FLAGS=
LIB_LUA=../../../lua-5.3.4/src/liblua.a
EXTRA_LD_FLAGS?=-lm -ldl
ADD_SANITIZERS_CC= -g -fsanitize=address -fno-omit-frame-pointer
ADD_SANITIZERS_LD= -g -fsanitize=address
MEM_SANITIZERS_CC= -g -fsanitize=memory -fno-omit-frame-pointer
MEM_SANITIZERS_LD= -g -fsanitize=memory
UB_SANITIZERS_CC= -g -fsanitize=undefined -fno-omit-frame-pointer
UB_SANITIZERS_LD= -g -fsanitize=undefined
COV_CC= -fprofile-instr-generate -fcoverage-mapping
COV_LD= -fprofile-instr-generate
# BUILD_MODES are=RELEASE(default), DEBUG,ADDSAN,MEMSAN,UBSAN
BUILD_MODE?=RELEASE
OBJ_LIST:=$(patsubst %.c, %.o, $(wildcard *.c))
ASM_LIST:=$(patsubst %.c, %.dis, $(wildcard *.c))

ifeq ($(BUILD_MODE), ADDSAN)
ifeq ($(CC), gcc)
$(error This build mode is only useable with clang.)
endif
CC_EXTRA+=$(ADD_SANITIZERS_CC)
EXTRA_LD_FLAGS+=$(ADD_SANITIZERS_LD)
endif

ifeq ($(BUILD_MODE), MEMSAN)
ifeq ($(CC), gcc)
$(error This build mode is only useable with clang.)
endif
CC_EXTRA+=$(MEM_SANITIZERS_CC)
EXTRA_LD_FLAGS+=$(MEM_SANITIZERS_LD)
endif

ifeq ($(BUILD_MODE), UBSAN)
ifeq ($(CC), gcc)
$(error This build mode is only useable with clang.)
endif
CC_EXTRA+=$(UB_SANITIZERS_CC)
EXTRA_LD_FLAGS+=$(UB_SANITIZERS_LD)
endif

SRCS:=$(wildcard *.c)
CC_FLAGS+=$(CC_EXTRA)
LD_FLAGS+=$(EXTRA_LD_FLAGS)

.DEFAULT:all

.PHONY:all clean help ASM SO TAGS

all:A ADBG

everything:$(TARGET) A ASM SO $(TARGET)-static $(TARGET)-dbg TAGS $(TARGET)-cov

depend:.depend

.depend:$(SRCS)
	rm -rf .depend
	$(CC) -MM $(CC_FLAGS) $^ > ./.depend
	echo $(patsubst %.o:, %.odbg:, $(shell $(CC) -MM $(CC_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\-\_]+\.odbg/\n&/g' >> ./.depend
	echo $(patsubst %.o:, %.ocov:, $(shell $(CC) -MM $(CC_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\-\_]+\.ocov/\n&/g' >> ./.depend

-include ./.depend

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

%.odbg:%.c
	$(CC) $(CC_FLAGS) -g -c $< -o $@

%.ocov:%.c
	$(CC) $(CC_FLAGS) $(COV_CC) -c $< -o $@

$(LIB_LUA):
	$(MAKE) -C ../../../lua-5.3.4/src linux

$(TARGET): $(TARGET).o $(LIB_LUA) read.o aggregate.o structs.o
	$(CC) $(LD_FLAGS) $^ -o $@

$(TARGET)-static: $(TARGET).o $(LIB_LUA) read.o aggregate.o structs.o
	$(CC) $^ $(LD_FLAGS) -static -o $@

$(TARGET)-dbg: $(TARGET).odbg $(LIB_LUA) read.odbg aggregate.odbg structs.odbg
	$(CC) $^ $(LD_FLAGS) -g -o $@

$(TARGET)-cov: $(TARGET).ocov $(LIB_LUA) read.ocov aggregate.ocov structs.ocov
	$(CC) $^ $(LD_FLAGS) $(COV_LD) -o $@

cov:
	@llvm-profdata merge -sparse ./default.profraw -o ./default.profdata
	@llvm-cov show $(TARGET)-cov -instr-profile=default.profdata

covrep:
	@llvm-profdata merge -sparse ./default.profraw -o ./default.profdata
	@llvm-cov report $(TARGET)-cov -instr-profile=default.profdata

ASM:$(ASM_LIST)

SO:$(TARGET).so

A:$(TARGET).a
ADBG:$(TARGET).adbg
ACOV:$(TARGET).acov

TAGS:tags

tags:$(SRCS)
	$(shell $(CC) -c -I $(CTAGS_I_PATH) -M $(SRCS)|\
		sed -e 's/[\\ ]/\n/g'|sed -e '/^$$/d' -e '/\.o:[ \t]*$$/d'|\
		ctags -L - --c++-kinds=+p --fields=+iaS --extra=+q)

%.dis: %.o
	objdump -r -d -M intel -S $< > $@

$(TARGET).so: $(TARGET).o read.o aggregate.o structs.o
	$(CC) $^ $(LD_FLAGS) -shared -o $@

$(TARGET).a: $(LIB_LUA) read.o aggregate.o structs.o
	ar rcs $(TARGET).a read.o aggregate.o structs.o

$(TARGET).adbg: $(LIB_LUA) read.odbg aggregate.odbg structs.odbg
	ar rcs $(TARGET).adbg read.odbg aggregate.odbg structs.odbg

$(TARGET).acov: $(LIB_LUA) read.ocov aggregate.ocov structs.ocov
	ar rcs $(TARGET).acov $(TARGET).ocov read.ocov aggregate.ocov structs.ocov

valgrind: $(TARGET)
	valgrind --leak-check=yes $(TARGET)

test: $(TARGET)
	$(TARGET)

clean:
	rm -f *.o *.dis *.odbg *.ocov *~ $(TARGET) $(TARGET).so $(TARGET)-static $(TARGET)-dbg $(TARGET).a $(TARGET)-cov $(TARGET).adbg $(TARGET).acov

deepclean:
	rm -f *.o *.dis *.odbg *.ocov *~ $(TARGET) $(TARGET).so tags $(TARGET)-static $(TARGET)-dbg $(TARGET).a $(TARGET)-cov
	rm .depend

help:
	@echo "--all is the default target, runs $(TARGET) target"
	@echo "--everything will build everything"
	@echo "--SO will generate the so"
	@echo "--ASM will generate assembly files"
	@echo "--TAGS will generate tags file"
	@echo "--$(TARGET) builds the dynamically-linked executable"
	@echo "--$(TARGET)-dbg will generate the debug build. BUILD_MODE should be set to DEBUG to work"
	@echo "--$(TARGET)-static will statically link the executable to the libraries"
	@echo "--$(TARGET)-cov is the coverage build"
	@echo "--cov will print the coverage report"
	@echo "--covrep will print the coverage report"
	@echo "--A will build the static library"
	@echo "--TAGS will build the tags file"
	@echo "--clean"
	@echo "--deepclean$(newline) will clean almost everything"
