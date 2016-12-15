
#######################################VARS####################################
#CXX=g++
CXX?=clang++

LLVM_CONF?=llvm-config

CXX_FLAGS=$(shell $(LLVM_CONF) --cxxflags)

#EXTRA_CXX_FALGS=-I/home/bloodstalker/extra/llvm-clang-4/llvm/tools/clang/include -I/home/bloodstalker/extra/llvm-clang-4/build/tools/clang/include
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell llvm-config --obj-root)/tools/clang/include -std=c++11
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o

LD_FLAGS=-Wl,--start-group -lclangAST -lclangAnalysis -lclangBasic\
-lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool\
-lclangLex -lclangParse -lclangSema -lclangEdit -lclangASTMatchers\
-lclangRewrite -lclangRewriteFrontend -lclangStaticAnalyzerFrontend\
-lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore\
-lclangSerialization -lclangToolingCore -lclangTooling -Wl,--end-group
LD_FLAGS+=$(shell llvm-config --ldflags --libs --system-libs)

CXX_FLAGS+=$(EXTRA_CXX_FALGS)
LD_FLAGS+=$(EXTRA_LD_FLAGS)

TARGET0=mutator-lvl0
TARGET=mutator
TARGET2=mutator-lvl2

######################################RULES####################################
.DEFAULT: all

.PHONY:all clean help

all: $(TARGET) $(TARGET2) $(TARGET0)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@
	$(MAKE) -C tinyxml2

$(TARGET): $(TARGET).o mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@

$(TARGET2): $(TARGET2).o mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@	

$(TARGET0): $(TARGET0).o mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(TARGET0) $(TARGET) $(TARGET2) 

help:
	@echo 'there is help.'
	@echo 'all is the default.'
	@echo 'clean.'
	@echo 'press tab for more targets if you have zsh!'
