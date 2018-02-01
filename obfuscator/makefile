
######################################INCLUDES#################################
include ../macros.mk

#######################################VARS####################################
OBSC=obfuscator
SRCS=$(wildcard *.cpp)
######################################RULES####################################
.DEFAULT: all

.PHONY: all clean help depend

all: $(OBSC)

depend: .depend

.depend:$(SRCS)
	rm -f ./.depend
	$(CXX) -MM $(CXX_FLAGS) $^ > ./.depend

-include ./.depend

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(OBSC): $(OBSC).o ../mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(OBSC)
	rm ./.depend

help:
	@echo 'There is help.'
	@echo 'All is the defualt target.'
	@echo 'Clean runs clean.'
	@echo 'For a more complete and detaild list of BUILD_MODE and other things look at the main makefiles help under project root.'
