BRUISER_TEST=test
CXX?=clang++
# you should change the include path to one that is right for you
CXX_FLAGS=-I/usr/lib/gcc/x86_64-redhat-linux/6.3.1/include

.DEFAULT:all clean

.PHONY:all clean $(BRUISER_TEST) help

all:$(BRUISER_TEST)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@ 

$(BRUISER_TEST): test.o
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(BRUISER_TEST)

help:
	@echo "the makfile for the bruiser test file."
	@echo "there is the taget itself, all and clean."

