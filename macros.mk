#MACRO DEFINITIONS FOR MUTATOR BUILDS

CXX=clang++
PY_CONF?=python3-config
CXX?=clang++
CC?=clang
LLVM_CONF?=llvm-config
BUILD_MODE?=COV_NO_CLANG_1Z
SHELL:=/bin/bash
MAKEFLAGS+=--warn-undefined-variables

ADD_SANITIZERS_CC= -g -fsanitize=address -fno-omit-frame-pointer
ADD_SANITIZERS_LD= -g -fsanitize=address
MEM_SANITIZERS_CC= -g -fsanitize=memory -fno-omit-frame-pointer
MEM_SANITIZERS_LD= -g -fsanitize=memory
UB_SANITIZERS_CC= -g  -fsanitize=undefined -fno-omit-frame-pointer
UB_SANITIZERS_LD= -g  -fsanitize=undefined

CXX_FLAGS=$(shell $(LLVM_CONF) --cxxflags)
CC_FLAGS=
EXTRA_CC_FLAGS=

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
 -std=c++17 -stdlib=libstdc++ -UNDEBUG -fexceptions
EXTRA_LD_FLAGS=-v
endif

##############################################################################################################################
ifeq ($(BUILD_MODE), ADDSAN)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++17 -stdlib=libstdc++ -UNDEBUG -fexceptions
EXTRA_CXX_FALGS+=$(ADD_SANITIZERS_CC)
EXTRA_LD_FLAGS=-v
EXTRA_LD_FLAGS+=$(ADD_SANITIZERS_LD)
endif

ifeq ($(BUILD_MODE), MEMSAN)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++17 -stdlib=libstdc++ -UNDEBUG -fexceptions
EXTRA_CXX_FALGS+=$(MEM_SANITIZERS_CC)
EXTRA_LD_FLAGS=-v
EXTRA_LD_FLAGS+=$(MEM_SANITIZERS_LD)
endif

ifeq ($(BUILD_MODE), UBSAN)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++17 -stdlib=libstdc++ -UNDEBUG -fexceptions
EXTRA_CXX_FALGS+=$(UB_SANITIZERS_CC)
EXTRA_LD_FLAGS=-v
EXTRA_LD_FLAGS+=$(UB_SANITIZERS_LD)
endif
##############################################################################################################################

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
-lclangSerialization -lclangToolingCore -lclangTooling -lstdc++ -lLLVMRuntimeDyld -lm -Wl,--end-group
LD_FLAGS+=$(shell $(LLVM_CONF) --ldflags --libs --system-libs)

CXX_FLAGS+=$(EXTRA_CXX_FALGS)
LD_FLAGS+=$(EXTRA_LD_FLAGS)
CC_FLAGS+=$(EXTRA_CC_FLAGS)

SRCS=./mutator_aux.cpp ./mutator-lvl1.cpp ./mutator-lvl0.cpp ./mutator-lvl2.cpp ./mutator-lvl0.h ./mutator_aux.h ./daemon/mutatord.h ./daemon/mutatorclient.c ./daemon/mutatorclient.h ./daemon/daemon_aux.h ./daemon/daemon_aux.c ./daemon/mutatord.c ./daemon/mutatorserver.c ./daemon/mutatorserver.h ./bruiser/bruiser.cpp ./bruiser/bruiser.h

CTAGS=ctags --c++-kinds=+p --fields=+iaS --extra=+q 
