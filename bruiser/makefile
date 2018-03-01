
######################################INCLUDES#################################
include ../macros.mk
#######################################VARS####################################
CXX_FLAGS+=-I/usr/include
CXX_FLAGS+=$(shell $(PY_CONF) --includes)
BRUISER=bruiser
LUA?=JIT
LIB_LUA=./lua-5.3.4/src/liblua.a
LIB_LUA_JIT=./LuaJIT/src/libluajit.a
HEADER_LIST=bruiser.h bruiser-extra.h CompletionHints.h
SRCS=$(wildcard *.cpp)
C_SRCS=$(wildcard *.c)
#for some reason without ld the build fails on ubuntu trusty on travis
#EXTRA_LD_FLAGS+=-lpthread -ldl -lutil -lm -Xlinker -lpython3
EXTRA_LD_FLAGS+=$(shell $(PY_CONF) --ldflags) -lffi -lcapstone -lkeystone
######################################RULES####################################
.DEFAULT: all

.PHONY: all clean help

all: $(BRUISER)

depend:.depend
dependc:.dependc

.depend:$(SRCS)
	$(CXX) -MM $(CXX_FLAGS) $^ > ./.depend

.dependc:$(C_SRCS)
	$(CC) -MM $(CC_FLAGS) $^ > ./.dependc

-include .depend
-include .dependc

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

linenoise.o:
	$(CC) $(CC_FLAGS) linenoise/linenoise.c -c -o linenoise.o

$(LIB_LUA):
	$(MAKE) -C lua-5.3.4 linux
	@echo "building with vanilla"

$(LIB_LUA_JIT):
	$(MAKE) -C LuaJIT
	@echo "building with jit"

$(BRUISER): $(BRUISER).o ../mutator_aux.o ../tinyxml2/tinyxml2.o linenoise.o CompletionHints.o mutagen.o ORCmutation.o bruiserffi.o bruisercapstone.o $(LIB_LUA)
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(BRUISER)
	rm .depend
	rm .dependc

deepclean:
	rm -f *.o *~ $(BRUISER)
	rm .depend
	rm .dependc
	$(MAKE) -C lua-5.3.4 clean
	$(MAKE) -C LuaJIT clean

help:
	@echo 'there is help.'
	@echo 'all is the defualt target.'
	@echo 'clean runs clean.'
	@echo 'deepclean will also clean the lua build'
	@echo 'for a more complete and detaild list of BUILD_MODE and other things look at the main makefiles help under project root.'
