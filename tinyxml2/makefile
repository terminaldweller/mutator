
include ../macros.mk
COV_CXX= -fprofile-instr-generate -fcoverage-mapping

.DEFAULT: tinyxml2.o

everything: tinyxml2.o tinyxml2.ocov tinyxml2.odbg

tinyxml2.o: tinyxml2.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@

tinyxml2.odbg: tinyxml2.cpp
	$(CXX) $(CXX_FLAGS) -g -c $< -o $@

tinyxml2.ocov: tinyxml2.cpp
	$(CXX) $(CXX_FLAGS) $(COV_CXX) -c $< -o $@

clean:
	rm -f *.o *.odbg *.ocov
