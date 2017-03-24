
######################################INCLUDES#################################
include ../macros.mk

#######################################VARS####################################
SFCPP01=safercpp-arr
######################################RULES####################################
.DEFAULT: all

.PHONY: all clean help $(SFCPP01)

all: $(SFCPP01)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(SFCPP01): $(SFCPP01).o ../mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(SFCPP01)

help:
	@echo 'There is help.'
	@echo 'All is the defualt target.'
	@echo 'Clean runs clean.'
	@echo 'For a more complete and detaild list of BUILD_MODE and other things look at the main makefiles help under project root.'
