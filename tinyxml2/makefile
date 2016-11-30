#CXX=g++
CXX=/home/bloodstalker/llvm/llvm/build/bin/clang++
CXX_FLAGS=$(shell /home/bloodstalker/llvm/llvm/build/bin/llvm-config --cxxflags)
EXTRA_CXX_FALGS=-I/home/bloodstalker/llvm/llvm/llvm/tools/clang/include -I/home/bloodstalker/llvm/llvm/build/tools/clang/include
CXX_FLAGS+=$(EXTRA_CXX_FALGS)

.DEFAULT: tinyxml2


tinyxml2.o: tinyxml2.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@	
