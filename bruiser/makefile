
######################################INCLUDES#################################
include ../macros.mk

#######################################VARS####################################
CXX_FLAGS+=-I/usr/include
EXTRA_LD_FLAGS+=-L$(shell pwd)/lua-5.3.4/src/ -llua -ldl
BRUISER=bruiser
LIB_LUA=./lua-5.3.4/src/liblua.a
######################################RULES####################################
.DEFAULT: all

.PHONY: all clean help $(BRUISER)

all: $(BRUISER)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@

linenoise.o:
	$(CC) $(CC_FLAGS) linenoise/linenoise.c -c -o linenoise.o

$(LIB_LUA):
	$(MAKE) -C lua-5.3.4 linux

$(BRUISER): $(BRUISER).o ../mutator_aux.o ../tinyxml2/tinyxml2.o linenoise.o CompletionHints.o $(LIB_LUA)
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(BRUISER)
	$(MAKE) -C lua-5.3.4 clean

help:
	@echo 'there is help.'
	@echo 'all is the defualt target.'
	@echo 'clean runs clean.'
	@echo 'for a more complete and detaild list of BUILD_MODE and other things look at the main makefiles help under project root.'
