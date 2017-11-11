
######################################INCLUDES#################################
include ../macros.mk

#######################################VARS####################################
OBSC=obfuscator
######################################RULES####################################
.DEFAULT: all

.PHONY: all clean help $(OBSC)

all: $(OBSC)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(OBSC): $(OBSC).o ../mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(OBSC)

help:
	@echo 'There is help.'
	@echo 'All is the defualt target.'
	@echo 'Clean runs clean.'
	@echo 'For a more complete and detaild list of BUILD_MODE and other things look at the main makefiles help under project root.'
