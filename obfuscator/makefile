
######################################INCLUDES#################################
include ../macros.mk

#######################################VARS####################################
OBSC=obfuscator
CXX_FLAGS+=-std=c++14
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
	$(CXX) -v $(CXX_FLAGS) -c $< -o $@

./keccak-tiny/.o:./keccak-tiny/.c
	$(CC) $(CC_FLAGS) -c $< -o $@

$(OBSC): $(OBSC).o ../mutator_aux.o ./keccak-tiny/keccak-tiny.o
	$(CXX) -v $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(OBSC) *.out ./keccak-tiny/*.o
	rm ./.depend

deepclean:
	rm -f *.o *~ $(OBSC) *.out FILE*.cpp FILE*.hpp ./keccak-tiny/*.o
	rm ./.depend

help:
	@echo 'There is help.'
	@echo 'All is the defualt target.'
	@echo 'Clean runs clean.'
	@echo 'For a more complete and detaild list of BUILD_MODE and other things look at the main makefiles help under project root.'
