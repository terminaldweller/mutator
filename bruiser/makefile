
######################################INCLUDES#################################
include ../macros.mk

#######################################VARS####################################
CXX_FLAGS+=-I/usr/include
EXTRA_LD_FLAGS+=-lncurses
BRUISER=bruiser

######################################RULES####################################
.DEFAULT: all

.PHONY: all clean help $(BRUISER)

all: $(BRUISER)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@

linenoise.o:
	$(CC) $(CC_FLAGS) linenoise/linenoise.c -c -o linenoise.o

lua:
	$(MAKE) -C lua-5.3.4

$(BRUISER): $(BRUISER).o ../mutator_aux.o ../tinyxml2/tinyxml2.o linenoise.o
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(BRUISER)

help:
	@echo 'There is help.'
	@echo 'All is the defualt target.'
	@echo 'Clean runs clean.'
	@echo 'For a more complete and detaild list of BUILD_MODE and other things look at the main makefiles help under project root.'
