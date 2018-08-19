
include macros.mk

TARGET0=mutator-lvl0
TARGET1=mutator-lvl1
TARGET2=mutator-lvl2
TARGETC=mutatorclient
TARGETD=mutatord
TARGETS=mutatorserver
SFCPP01=safercpp-arr
BRUISER=bruiser
OBSC=obfuscator

.DEFAULT: all

.PHONY:all clean install help $(BRUISER) $(OBSC) $(TARGETC) $(TARGETD) $(TARGETS) $(SFCPP01)

all: $(TARGET0) $(TARGETC) $(TARGETD) $(TARGETS) $(SFCPP01) $(BRUISER) $(OBSC)

$(TARGET1):
	$(CXX) $^ $(LD_FLAGS) -o $@

$(TARGET2):
	$(CXX) $^ $(LD_FLAGS) -o $@

$(TARGET0):
	$(MAKE) -C m0 CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)

$(SFCPP01):
	$(MAKE) -C safercpp CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)

$(BRUISER):
	$(MAKE) -C bruiser CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)

$(OBSC):
	$(MAKE) -C obfuscator CXX=$(CXX) LLVM_CONF=$(LLVM_CONF) BUILD_MODE=$(BUILD_MODE)

$(TARGETC):
	$(MAKE) -C daemon mutatorclient

$(TARGETD):
	$(MAKE) -C daemon mutatord

$(TARGETS):
	$(MAKE) -C daemon mutatorserver

clean:
	$(MAKE) -C tinyxml2 clean
	$(MAKE) -C json clean
	$(MAKE) -C daemon clean
	$(MAKE) -C safercpp clean
	$(MAKE) -C bruiser clean
	$(MAKE) -C obfuscator clean
	$(MAKE) -C m0 clean

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
	if [[ ! -d "./reliquary/obfuscator" ]]; then mkdir ./reliquary/obfuscator; fi
	$(shell echo MUTATOR_HOME=$$(pwd) > ./daemon/mutator.config)

help:
	@echo "Under Construction"
