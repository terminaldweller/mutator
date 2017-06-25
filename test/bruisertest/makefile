BRUISER_TEST=test
CXX?=clang++

.DEFAULT:all clean

all:$(BRUISER_TEST)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@ -I/usr/include

$(BRUISER_TEST): test.cpp
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(BRUISER_TEST)

help:
	@echo "the makfile for the bruiser test file."
	@echo "there is the taget itself, all and clean."

