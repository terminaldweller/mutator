
######################################INCLUDES#################################
include ../macros.mk

#######################################VARS####################################
CXX_FLAGS+=
BRUISER=bruiser

######################################RULES####################################
.DEFAULT: all

.PHONY: all clean help $(BRUISER)

all: $(BRUISER)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(BRUISER): $(BRUISER).o ../mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(BRUISER)

help:
	@echo 'There is help.'
	@echo 'All is the defualt target.'
	@echo 'Clean runs clean.'
	@echo 'For a more complete and detaild list of BUILD_MODE and other things look at the main makefiles help under project root.'
