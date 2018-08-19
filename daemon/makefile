CC=clang
CC?=clang
SHELL=bash
SHELL?=bash
CC_FLAGS=-fpic
LD_FLAGS=
TARGETD=mutatord
TARGETS=mutatorserver
TARGETC=mutatorclient
COV_CC= -fprofile-instr-generate -fcoverage-mapping
COV_LD= -fprofile-instr-generate
SRCS:=$(wildcard *.c)
OBJ_LIST:=$(patsubst %.c, %.o, $(wildcard *.c))
ASM_LIST:=$(patsubst %.c, %.dis, $(wildcard *.c))
CTAGS_I_PATH?=./

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

CC_FLAGS+=$(CC_EXTRA)
LD_FLAGS+=$(EXTRA_LD_FLAGS)

.DEFAULT:all

.PHONY:all clean help

all:$(TARGETC) $(TARGETS) $(TARGETD)

everything: $(TARGETC) $(TARGETD) $(TARGETS) $(TARGETC)-dbg $(TARGETD)-dbg $(TARGETS)-dbg $(TARGETC)-cov $(TARGETD)-cov $(TARGETS)-cov ASM TAGS $(TARGETC)-static $(TARGETD)-static $(TARGETS)-static

depend:.depend

.depend:$(SRCS)
	rm -rf .depend
	$(CC) -MM $(CC_FLAGS) $^ > ./.depend
	echo $(patsubst %.o:, %.odbg:, $(shell $(CC) -MM $(CC_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\-\_]+\.odbg/\n&/g' >> ./.depend
	echo $(patsubst %.o:, %.ocov:, $(shell $(CC) -MM $(CC_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\-\_]+\.ocov/\n&/g' >> ./.depend

-include .depend

ASM: $(ASM_LIST)

TAGS: tags

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

%.odbg:%.c
	$(CC) $(CC_FLAGS) -g -c $< -o $@

%.ocov:%.c
	$(CC) $(CC_FLAGS) $(COV_CC) -c $< -o $@

$(TARGETC): $(TARGETC).o
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGETS): $(TARGETS).o daemon_aux.o
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGETD): $(TARGETD).o daemon_aux.o
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGETC)-dbg: $(TARGETC).odbg
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGETS)-dbg: $(TARGETS).odbg daemon_aux.odbg
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGETD)-dbg: $(TARGETS).odbg daemon_aux.odbg
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGETC)-cov: $(TARGETC).ocov
	$(CC) $^ $(LD_FLAGS) $(COV_LD) -o $@

$(TARGETS)-cov: $(TARGETS).ocov daemon_aux.ocov
	$(CC) $^ $(LD_FLAGS) $(COV_LD) -o $@

$(TARGETD)-cov: $(TARGETD).ocov daemon_aux.ocov
	$(CC) $^ $(LD_FLAGS) $(COV_LD) -o $@

$(TARGETC)-static: $(TARGETC).o
	$(CC) $^ $(LD_FLAGS) -static -o $@

$(TARGETS)-static: $(TARGETS).o daemon_aux.o
	$(CC) $^ $(LD_FLAGS) -static -o $@

$(TARGETD)-static: $(TARGETD).o daemon_aux.o
	$(CC) $^ $(LD_FLAGS) -static -o $@

%.dis: %.o
	objdump -r -d -M intel -S $< > $@

tags:$(SRCS)
	$(shell $(CC) -c -I $(CTAGS_I_PATH) -M $(SRCS)|\
		sed -e 's/[\\ ]/\n/g'|sed -e '/^$$/d' -e '/\.o:[ \t]*$$/d'|\
		ctags -L - --c++-kinds=+p --fields=+iaS --extra=+q)

cov:
	@llvm-profdata merge -sparse ./default.profraw -o ./default.profdata
	@llvm-cov show $(TARGET)-cov -instr-profile=default.profdata

covrep:
	@llvm-profdata merge -sparse ./default.profraw -o ./default.profdata
	@llvm-cov report $(TARGET)-cov -instr-profile=default.profdata

clean:
	rm ./.depend
	rm -f *.o *~ $(TARGETD) $(TARGETS) $(TARGETC) *.ocov *.odbg $(TARGETD)-cov $(TARGETC)-cov $(TARGETS)-cov $(TARGETD)-dbg $(TARGETC)-dbg $(TARGETS)-dbg tags $(TARGETD)-static $(TARGETC)-static $(TARGETS)-static

help:
	@echo 'all builds the daemon, the server and the client. all is the default.'
	@echo 'mutatord builds the daemon with the server'
	@echo 'mutatorc builds the client'
	@echo 'mutators builds the standalone server(deprecated)'
	@echo 'clean'
	@echo 'help'
