
######################################INCLUDES#################################
include macros.mk

#######################################VARS####################################
EXTRA_LD_FLAGS+=tinyxml2/tinyxml2.o


TARGET0=mutator-lvl0
TARGET1=mutator-lvl1
TARGET2=mutator-lvl2
TARGETC=mutatorclient
TARGETD=mutatord
TARGETS=mutatorserver
SFCPP01=safercpp-arr
BRUISER=bruiser

######################################RULES####################################
.DEFAULT: all

.PHONY:all clean install help $(TARGET0) $(TARGET1) $(TARGET2) TAGS $(SFCPP01) $(BRUISER)

all: $(TARGET0) $(TARGET1) $(TARGET2) $(TARGETC) $(TARGETD) $(TARGETS) $(SFCPP01) $(BRUISER)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $< -o $@
	$(MAKE) -C tinyxml2 CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)
	$(MAKE) -C json CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)

$(TARGET1): $(TARGET1).o mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@

$(TARGET2): $(TARGET2).o mutator_aux.o
	$(CXX) $^ $(LD_FLAGS) -o $@	

$(TARGET0): $(TARGET0).o mutator_aux.o mutator_report.o
	$(CXX) $^ $(LD_FLAGS) -o $@

$(SFCPP01): ./safercpp/$(SFCPP01).o mutator_aux.o
	$(MAKE) -C safercpp CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)

$(BRUISER): ./bruiser/$(BRUISER).o mutator_aux.o
	$(MAKE) -C bruiser CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)

$(TARGETC):
	$(MAKE) -C daemon mutatorclient

$(TARGETD):
	$(MAKE) -C daemon mutatord

$(TARGETS):
	$(MAKE) -C daemon mutatorserver

TAGS: $(SRCS)
	$(CTAGS) $(SRCS)

clean:
	rm -f *.o *~ $(TARGET0) $(TARGET1) $(TARGET2)
	$(MAKE) -C tinyxml2 clean
	$(MAKE) -C json clean
	$(MAKE) -C daemon clean
	$(MAKE) -C safercpp clean
	$(MAKE) -C bruiser clean

install:
	chmod +x ./mutator.sh
	chmod +x ./extra-tools/ReportPrintPretty.sh
	chmod +x ./extra-tools/precommitTests.sh
	chmod +x ./extra-tools/oracle.sh
	if [[ ! -d "./temp" ]]; then mkdir temp; fi
	if [[ ! -d "./reliquary" ]]; then mkdir reliquary; fi
	if [[ ! -d "./reliquary/m0" ]]; then mkdir ./reliquary/m0; fi
	if [[ ! -d "./reliquary/bruiser" ]]; then mkdir ./reliquary/bruiser; fi
	if [[ ! -d "./reliquary/safercpp" ]]; then mkdir ./reliquary/safercpp; fi
	$(shell echo MUTATOR_HOME=$$(pwd) > ./daemon/mutator.config)

help:
	@echo '- There is help.'
	@echo '- All is the default.'
	@echo '- install makes the scripts executable. Also creates the reliquary.'
	@echo '- Clean.'
	@echo '- TAGS will run ctags on the C/C++ source files.'
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
