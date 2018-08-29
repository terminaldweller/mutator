TARGET=bruiser
LLVM_CONF?=llvm-config
PY_CONF?=python3-config
LIB_LUA=./lua-5.3.4/src/liblua.a
SHELL=bash
SHELL?=bash
CC=clang
CC?=clang
CC_FLAGS=-fpic -std=c11 $(shell $(PY_CONF) --includes)
CXX=clang++
CXX?=clang++
CXX_FLAGS=-fpic -I/usr/include $(shell $(PY_CONF) --includes)
CXX_EXTRA?=
CTAGS_I_PATH?=./
LD_FLAGS=
EXTRA_LD_FLAGS?=
ADD_SANITIZERS_CC= -g -fsanitize=address -fno-omit-frame-pointer
ADD_SANITIZERS_LD= -g -fsanitize=address
MEM_SANITIZERS_CC= -g -fsanitize=memory -fno-omit-frame-pointer
MEM_SANITIZERS_LD= -g -fsanitize=memory
UB_SANITIZERS_CC= -g -fsanitize=undefined -fno-omit-frame-pointer
UB_SANITIZERS_LD= -g -fsanitize=undefined
COV_CXX= -fprofile-instr-generate -fcoverage-mapping
COV_LD= -fprofile-instr-generate
# BUILD_MODES are=RELEASE(default), DEBUG,ADDSAN,MEMSAN,UBSAN
BUILD_MODE?=RELEASE
OBJ_LIST:=$(patsubst %.cpp, %.o, $(wildcard *.cpp))
ASM_LIST:=$(patsubst %.cpp, %.dis, $(wildcard *.cpp))

LLVM_CXX_FLAGS=$(shell $(LLVM_CONF) --cxxflags)
LLVM_CXX_FLAGS+=-I$(shell $(LLVM_CONF) --src-root)/tools/clang/include\
 -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include\
 -stdlib=libstdc++ -std=c++17 -fexceptions
LLVM_LD_FLAGS=-Wl,--start-group -lclangAST -lclangAnalysis -lclangBasic\
 -lclangDriver -lclangEdit -lclangFrontend -lclangFrontendTool\
 -lclangLex -lclangParse -lclangSema -lclangEdit -lclangASTMatchers\
 -lclangRewrite -lclangRewriteFrontend -lclangStaticAnalyzerFrontend\
 -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore\
 -lclangSerialization -lclangToolingCore -lclangTooling -lstdc++\
 -lLLVMRuntimeDyld -lm -Wl,--end-group
LLVM_LD_FLAGS+=$(shell $(LLVM_CONF) --ldflags --libs --system-libs)

CXX_FLAGS+=$(LLVM_CXX_FLAGS)
LD_FLAGS+=$(LLVM_LD_FLAGS)

#MAKEFLAGS+=--warn-undefined-variables
ifeq ($(BUILD_MODE), ADDSAN)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
CXX_EXTRA+=$(ADD_SANITIZERS_CC)
EXTRA_LD_FLAGS+=$(ADD_SANITIZERS_LD)
endif

ifeq ($(BUILD_MODE), MEMSAN)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
CXX_EXTRA+=$(MEM_SANITIZERS_CC)
EXTRA_LD_FLAGS+=$(MEM_SANITIZERS_LD)
endif

ifeq ($(BUILD_MODE), UBSAN)
ifeq ($(CXX), g++)
$(error This build mode is only useable with clang++.)
endif
CXX_EXTRA+=$(UB_SANITIZERS_CC)
EXTRA_LD_FLAGS+=$(UB_SANITIZERS_LD)
endif

SRCS:=$(wildcard *.cpp)
C_SRCS:=$(wildcard *.c)
CXX_FLAGS+=$(CXX_EXTRA)
EXTRA_LD_FLAGS+=$(shell $(PY_CONF) --ldflags) -lffi -lcapstone -lkeystone -L./lua-5.3.4/src
LD_FLAGS+=$(EXTRA_LD_FLAGS)
TBG_OBJLIST_INC=$(patsubst ./luatablegen/%.c, ./luatablegen/%.o, $(wildcard ./luatablegen/*.c))
TBG_OBJLIST_DBG_INC=$(patsubst ./luatablegen/%.c, ./luatablegen/%.odbg, $(wildcard ./luatablegen/*.c))
TBG_OBJLIST_COV_INC=$(patsubst ./luatablegen/%.c, ./luatablegen/%.ocov, $(wildcard ./luatablegen/*.c))

.DEFAULT:all

.PHONY:all clean help ASM SO TAGS

all:$(TARGET)

everything:$(TARGET) A ASM SO $(TARGET)-dbg TAGS $(TARGET)-cov

depend:.depend
dependc:.dependc

.depend:$(SRCS)
	rm -rf .depend
	$(CXX) -MM $(CXX_FLAGS) $^ > ./.depend
	echo $(patsubst %.o:, %.odbg:, $(shell $(CXX) -MM $(CXX_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\_]+\.odbg/\n&/g' >> ./.depend
	echo $(patsubst %.o:, %.ocov:, $(shell $(CXX) -MM $(CXX_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\_]+\.ocov/\n&/g' >> ./.depend

.dependc:$(C_SRCS)
	rm -rf .dependc
	$(CC) -MM $(CC_FLAGS) $^ > ./.dependc
	echo $(patsubst %.o:, %.odbg:, $(shell $(CC) -MM $(CC_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\_]+\.odbg/\n&/g' >> ./.dependc
	echo $(patsubst %.o:, %.ocov:, $(shell $(CC) -MM $(CC_FLAGS) $^)) | sed -r 's/[A-Za-z0-9\_]+\.ocov/\n&/g' >> ./.dependc

-include ./.depend
-include ./.dependc

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

%.ocov:%.c
	$(CC) $(CC_FLAGS) $(COV_CXX) -c $< -o $@

%.odbg:%.c
	$(CC) $(CC_FLAGS) -g -c $< -o $@

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@

#../mutator_aux.o:../mutator_aux.cpp
	#$(CXX) $(CXX_FLAGS) -c $< -o $@

linenoise.o: ./linenoise/linenoise.c
	$(CC) $(CC_FLAGS) linenoise/linenoise.c -c -o linenoise.o

linenoise.odbg: ./linenoise/linenoise.c
	$(CC) $(CC_FLAGS) linenoise/linenoise.c -g -c -o linenoise.odbg

linenoise.ocov: ./linenoise/linenoise.c
	$(CC) $(CC_FLAGS) $(COV_CXX) linenoise/linenoise.c -c -o linenoise.ocov

./wasmtablegen.json:
	if [[ ls -l ./luatablegen | wc -l  == 2 ]];then ./tablegen.sh;else :;fi

./luatablegen/%.c: ./wasmtablegen.json
	./tablegen.sh

./luatablegen/%.o:./luatablegen/%.c
	$(MAKE) -C luatablegen

$(LIB_LUA):
	$(MAKE) -C lua-5.3.4/src linux

%.odbg:%.cpp
	$(CXX) $(CXX_FLAGS) -g -c $< -o $@

%.ocov:%.cpp
	$(CXX) $(CXX_FLAGS) $(COV_CXX) -c $< -o $@

$(TARGET): $(TARGET).o ../m0/mutator_aux.o ../tinyxml2/tinyxml2.o linenoise.o CompletionHints.o mutagen.o ORCmutation.o bruiserffi.o asmrewriter.o bruisercapstone.o ramdump.o ffs.o $(LIB_LUA) $(TBG_OBJLIST_INC)
	$(CXX) $^ $(LD_FLAGS) -o $@

# currently broken since it needs a static libpython
$(TARGET)-static: $(TARGET).o ../m0/mutator_aux.o ../tinyxml2/tinyxml2.o linenoise.o CompletionHints.o mutagen.o ORCmutation.o bruiserffi.o asmrewriter.o bruisercapstone.o ramdump.o ffs.o $(LIB_LUA) $(TBG_OBJLIST_INC)
	$(CXX) $^ $(LD_FLAGS) -static -o $@

$(TARGET)-dbg: $(TARGET).odbg ../m0/mutator_aux.odbg ../tinyxml2/tinyxml2.odbg linenoise.odbg CompletionHints.odbg mutagen.o ORCmutation.o bruiserffi.odbg asmrewriter.odbg bruisercapstone.odbg ramdump.odbg ffs.odbg $(LIB_LUA) $(TBG_OBJLIST_DBG_INC)
	$(CXX) $^ $(LD_FLAGS) -g -o $@

$(TARGET)-cov: $(TARGET).ocov ../m0/mutator_aux.ocov ../tinyxml2/tinyxml2.ocov linenoise.ocov CompletionHints.ocov mutagen.o ORCmutation.o bruiserffi.ocov asmrewriter.ocov bruisercapstone.ocov ramdump.ocov ffs.ocov $(LIB_LUA) $(TBG_OBJLIST_COV_INC)
	$(CXX) $^ $(LD_FLAGS) $(COV_LD) -o $@

cov:
	@llvm-profdata merge -sparse ./default.profraw -o ./default.profdata
	@llvm-cov show $(TARGET)-cov -instr-profile=default.profdata

covrep:
	@llvm-profdata merge -sparse ./default.profraw -o ./default.profdata
	@llvm-cov report $(TARGET)-cov -instr-profile=default.profdata

ASM:$(ASM_LIST)

SO:$(TARGET).so

A:$(TARGET).a

TAGS:tags

tags:$(SRCS)
	$(shell $(CXX) -c $(shell $(PY_CONF) --includes) $(shell $(LLVM_CONF) --cxxflags) -I$(shell $(LLVM_CONF)\
		--src-root)/tools/clang/include -I$(shell $(LLVM_CONF) --obj-root)/tools/clang/include -I $(CTAGS_I_PATH) -M $(SRCS)|\
		sed -e 's/[\\ ]/\n/g'|sed -e '/^$$/d' -e '/\.o:[ \t]*$$/d'|\
		ctags -L - --c++-kinds=+p --fields=+iaS --extra=+q)

%.dis: %.o
	objdump -r -d -M intel -S $< > $@

$(TARGET).so: $(TARGET).o ../m0/mutator_aux.o ../tinyxml2/tinyxml2.o linenoise.o CompletionHints.o mutagen.o ORCmutation.o bruiserffi.o asmrewriter.o bruisercapstone.o ramdump.o ffs.o $(LIB_LUA) $(TBG_OBJLIST_INC)
	$(CXX) $^ $(LD_FLAGS) -shared -o $@

$(TARGET).a: $(TARGET).o ../m0/mutator_aux.o ../tinyxml2/tinyxml2.o linenoise.o CompletionHints.o mutagen.o ORCmutation.o bruiserffi.o asmrewriter.o bruisercapstone.o ramdump.o ffs.o $(LIB_LUA) $(TBG_OBJLIST_INC)
	ar rcs $(TARGET).a $(TARGET).o

runcov: $(TARGET)-cov
	$(TARGET)-cov --lua ./lua-scripts/regtest.lua

valgrind: $(TARGET)
	valgrind --leak-check=yes $(TARGET) --lua ./lua-scripts/regtest.lua

clean:
	rm -f *.o *.dis *.odbg *.ocov *~ $(TARGET) $(TARGET).so $(TARGET)-static $(TARGET)-dbg $(TARGET).a $(TARGET)-cov

deepclean: clean
	rm tags
	rm .depend
	$(MAKE) -C lua-5.3.4 clean
	$(MAKE) -C luatablegen clean

help:
	@echo "--all is the default target, runs $(TARGET) target"
	@echo "--everything will build everything"
	@echo "--SO will generate the so"
	@echo "--ASM will generate assembly files"
	@echo "--TAGS will generate tags file"
	@echo "--$(TARGET) builds the dynamically-linked executable"
	@echo "--$(TARGET)-dbg will generate the debug build. BUILD_MODE should be set to DEBUG to work"
	@echo "--$(TARGET)-static will statically link the executable to the libraries"
	@echo "--$(TARGET)-cov is the coverage build"
	@echo "--cov will print the line coverage report"
	@echo "--covrep will print the coverage report"
	@echo "--A will build the static library"
	@echo "--TAGS will build the tags file"
	@echo "--clean"
	@echo "--deepclean will clean almost everything"
