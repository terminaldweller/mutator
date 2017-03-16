
######################################INCLUDES#################################
include ../macros.mk

#######################################VARS####################################
CXX?=clang++
LLVM_CONF?=llvm-config
BUILD_MODE?=COV_NO_CLANG_1Z
SHELL:=/bin/bash

CXX_FLAGS=$(shell $(LLVM_CONF) --cxxflags)

ifeq ($(BUILD_MODE), COV_USE)
ifneq ($(CXX), clang++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fprofile-instr-use=code.profdata -fexceptions
EXTRA_LD_FLAGS=-v -fprofile-instr-use=code.profdata
endif

ifeq ($(BUILD_MODE), COV_GEN)
ifneq ($(CXX), clang++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fprofile-instr-generate -fexceptions
EXTRA_LD_FLAGS=-v -fprofile-instr-generate
endif

#for gcov compatibility
ifeq ($(BUILD_MODE), COV_GNU)
#ifneq ($(CXX), clang++)
#$(error This build mode is only useable with clang++.)
#endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -UNDEBUG -fprofile-arcs -ftest-coverage -fexceptions -Xclang -coverage-version='408*' -Xclang -coverage-cfg-checksum\
 -Xclang -coverage-no-function-names-in-data
EXTRA_LD_FLAGS=-v -fprofile-arcs -ftest-coverage -fexceptions -Xclang -coverage-version='408*' -Xclang -coverage-cfg-checksum\
 -Xclang -coverage-no-function-names-in-data
endif

ifeq ($(BUILD_MODE), COV_NO_CLANG)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v
endif

ifeq ($(BUILD_MODE), WIN_BUILD)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v
endif

ifeq ($(BUILD_MODE), COV_NO_CLANG_1Z)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++1z -stdlib=libstdc++ -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v
endif

ifeq ($(BUILD_MODE), COV_NO_CLANG_14)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++14 -stdlib=libstdc++ -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v
endif

ifeq ($(BUILD_MODE), GNU_MODE)
ifneq ($(CXX), g++)
$(error This build mode is only useable with g++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -static-libstdc++ -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v
endif

LD_FLAGS=-Wl,--start-group -lclangAST -lclangAnalysis -lclangBasic\
-lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool\
-lclangLex -lclangParse -lclangSema -lclangEdit -lclangASTMatchers\
-lclangRewrite -lclangRewriteFrontend -lclangStaticAnalyzerFrontend\
-lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore\
-lclangSerialization -lclangToolingCore -lclangTooling -lstdc++ -lLLVMRuntimeDyld  -Wl,--end-group
LD_FLAGS+=$(shell $(LLVM_CONF) --ldflags --libs --system-libs)

CXX_FLAGS+=$(EXTRA_CXX_FALGS)
LD_FLAGS+=$(EXTRA_LD_FLAGS)

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
