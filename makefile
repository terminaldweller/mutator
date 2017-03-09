
######################################INCLUDES#################################
include macros.mk

#######################################VARS####################################
CXX?=clang++
CC?=clang
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
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o -fprofile-instr-use=code.profdata
endif

ifeq ($(BUILD_MODE), COV_GEN)
ifneq ($(CXX), clang++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fprofile-instr-generate -fexceptions
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o -fprofile-instr-generate
endif

#for gcov compatibility
ifeq ($(BUILD_MODE), COV_GNU)
#ifneq ($(CXX), clang++)
#$(error This build mode is only useable with clang++.)
#endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -UNDEBUG -fprofile-arcs -ftest-coverage -fexceptions -Xclang -coverage-version='408*' -Xclang -coverage-cfg-checksum\
 -Xclang -coverage-no-function-names-in-data
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o -fprofile-arcs -ftest-coverage -fexceptions -Xclang -coverage-version='408*' -Xclang -coverage-cfg-checksum\
 -Xclang -coverage-no-function-names-in-data
endif

ifeq ($(BUILD_MODE), COV_NO_CLANG)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
endif

ifeq ($(BUILD_MODE), WIN_BUILD)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
endif

ifeq ($(BUILD_MODE), COV_NO_CLANG_1Z)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++1z -stdlib=libstdc++ -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
endif

ifeq ($(BUILD_MODE), GNU_MODE)
ifneq ($(CXX), g++)
$(error This build mode is only useable with g++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -static-libstdc++ -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
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

TARGET0=mutator-lvl0
TARGET1=mutator-lvl1
TARGET2=mutator-lvl2
TARGETC=mutatorclient
TARGETD=mutatord
TARGETS=mutatorserver

######################################RULES####################################
.DEFAULT: all

.PHONY:all clean install help $(TARGET0) $(TARGET1) $(TARGET2)

all: $(TARGET0) $(TARGET1) $(TARGET2) $(TARGETC) $(TARGETD) $(TARGETS)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@
	$(MAKE) -C tinyxml2 CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)
	$(MAKE) -C json CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)

$(TARGET1): $(TARGET1).o mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@

$(TARGET2): $(TARGET2).o mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@	

$(TARGET0): $(TARGET0).o mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@

$(TARGETC):
	$(MAKE) -C daemon mutatorclient

$(TARGETD):
	$(MAKE) -C daemon mutatord

$(TARGETS):
	$(MAKE) -C daemon mutatorserver

clean:
	rm -f *.o *~ $(TARGET0) $(TARGET1) $(TARGET2)
	$(MAKE) -C tinyxml2 clean
	$(MAKE) -C json clean
	$(MAKE) -C daemon clean

install:
	chmod +x ./mutator.sh
	chmod +x ./extra-tools/ReportPrintPretty.sh
	chmod +x ./extra-tools/precommitTests.sh
	if [[ ! -d "./temp" ]]; then mkdir temp; fi

help:
	@echo '- There is help.'
	@echo '- All is the default.'
	@echo '- install makes the scripts executable. Currently this is all it does.'
	@echo '- Clean.'
	@echo '- You can use the target names as build targets to just build one executable.'
	@echo '- LLVM_CONF will tell the makefile the name of llvm-config. llvm-config is the default.'
	@echo '- CXX will let you set the compiler. currently the only accepted values are clang++ and g++. clang++ is the default.'
	@echo '- BUILD_MODE will let you choose to build for different coverage formats. the default is COV_NO_CLANG. the supported values are:'
	@echo '		COV_USE: adds the clang -fprofile-instr-use option(clang++ only mode).'
	@echo '		COV_GEN: adds the clang -fprofile-instr-generate option(clang++ only mode).'
	@echo '		COV_GNU: generates coverage for the build compatible with gcov(clang++ only mode).'
	@echo '		COV_NO_CLANG: this build mode will not support any coverage format and is meant to be used with clang++(clang++ only mode).'
	@echo '		COV_NO_CLANG_1Z: does not instrument the code for any coverage and uses -std=c++1z (clang++ only mode).'
	@echo '		GNU_MODE: meant to be used for builds with g++. supports no coverage(g++ only mode).'
	@echo '		WIN_MODE: to support windows builds'
	@echo '- Press tab for more targets if you have zsh!'
