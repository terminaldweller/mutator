
include ../macros.mk
COV_CXX= -fprofile-instr-generate -fcoverage-mapping

.DEFAULT: json.o

everything: json.o json.ocov json.odbg

json.o: json.hpp
	$(CXX) $(CXX_FLAGS) -x c++ -c $< -o $@

json.odbg: json.hpp
	$(CXX) $(CXX_FLAGS) -x c++ -g -c $< -o $@

json.ocov: json.hpp
	$(CXX) $(CXX_FLAGS) $(COV_CXX) -x c++ -c $< -o $@

clean:
	rm -f *.o *.odbg *.ocov
