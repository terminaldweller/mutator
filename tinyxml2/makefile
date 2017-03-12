
#############################################################VARS#############################################################
CXX?=clang++
LLVM_CONF?=llvm-config
BUILD_MODE?=COV_NO_CLANG
CXX_FLAGS=$(shell $(LLVM_CONF) --cxxflags)

ifeq ($(BUILD_MODE), COV_USE)
ifneq ($(CXX), clang++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fprofile-instr-use=code.profdata
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o -fprofile-instr-use=code.profdata
endif

ifeq ($(BUILD_MODE), COV_GEN)
ifneq ($(CXX), clang++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG -fprofile-instr-generate
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o -fprofile-instr-generate
endif

#for gcov compatibility
ifeq ($(BUILD_MODE), COV_GNU)
#ifneq ($(CXX), clang++)
#$(error This build mode is only useable with clang++.)
#endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -UNDEBUG -fprofile-arcs -ftest-coverage
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o -fprofile-arcs -ftest-coverage
endif

ifeq ($(BUILD_MODE), COV_NO_CLANG)
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -stdlib=libstdc++ -UNDEBUG
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
endif

ifeq ($(BUILD_MODE), COV_NO_CLANG_1Z)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++1z -stdlib=libstdc++ -UNDEBUG
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
endif

ifeq ($(BUILD_MODE), COV_NO_CLANG_14)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++14 -stdlib=libstdc++ -UNDEBUG
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
endif

ifeq ($(BUILD_MODE), GNU_MODE)
ifneq ($(CXX), g++)
$(error This build mode is only useable with g++.)
endif
EXTRA_CXX_FALGS=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -std=c++11 -static-libstdc++ -UNDEBUG
EXTRA_LD_FLAGS=-v tinyxml2/tinyxml2.o
endif

CXX_FLAGS+=$(EXTRA_CXX_FALGS)

###########################################################RULES##############################################################
.DEFAULT: tinyxml2

.PHONY: tinyxml2

tinyxml2.o: tinyxml2.cpp
	$(CXX) $(CXX_FLAGS) -c $< -o $@	

clean:
	rm -f *.o
