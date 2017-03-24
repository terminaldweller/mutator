
######################################INCLUDES#################################
include ../macros.mk

######################################RULES####################################
.DEFAULT: tinyxml2

.PHONY: tinyxml2

tinyxml2.o: tinyxml2.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@	

clean:
	rm -f *.o
