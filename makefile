
#######################################VARS####################################
CXX?=clang++
LLVM_CONF?=llvm-config
BUILD_MODE?=COV_NO_CLANG

CXX_FLAGS=$(shell $(LLVM_CONF) --cxxflags)

ifeq ($(BUILD_MODE), COV_USE)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fprofile-instr-use=code.profdata
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o -fprofile-instr-use=code.profdata
endif

ifeq ($(BUILD_MODE), COV_GEN)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fprofile-instr-generate
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o -fprofile-instr-generate
endif

#for gcov compatibility
ifeq ($(BUILD_MODE), COV_GNU)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fprofile-arcs -ftest-coverage
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o -fprofile-arcs -ftest-coverage
endif

ifeq ($(BUILD_MODE), COV_NO_CLANG)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
endif

ifeq ($(BUILD_MODE), GNU_MODE)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
endif

LD_FLAGS=-Wl,--start-group -lclangAST -lclangAnalysis -lclangBasic\
-lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool\
-lclangLex -lclangParse -lclangSema -lclangEdit -lclangASTMatchers\
-lclangRewrite -lclangRewriteFrontend -lclangStaticAnalyzerFrontend\
-lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore\
-lclangSerialization -lclangToolingCore -lclangTooling -lstdc++ -Wl,--end-group
LD_FLAGS+=$(shell $(LLVM_CONF) --ldflags --libs --system-libs)

CXX_FLAGS+=$(EXTRA_CXX_FALGS)
LD_FLAGS+=$(EXTRA_LD_FLAGS)

TARGET0=mutator-lvl0
TARGET=mutator
TARGET2=mutator-lvl2

######################################RULES####################################
.DEFAULT: all

.PHONY:all clean help $(TARGET) $(TARGET0) $(TARGET2)

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
