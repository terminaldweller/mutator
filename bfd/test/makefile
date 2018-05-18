
##################################VARS#################################
CC?=gcc
CC=gcc
CC_FLAGS=-fpic -O0 -g -v --debug
#LD_FLAGS= -l bfd
LD_FLAGS=
TARGET=test
##################################RULES################################
.DEFAULT:all

.PHONY:all clean help

all:$(TARGET) $(TARGET).so $(TARGET).asm

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

$(TARGET): $(TARGET).o
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGET).asm: $(TARGET).o
	objdump -r -d -M intel -S $(TARGET).o > $(TARGET).asm

$(TARGET).so: $(TARGET).o
	$(CC) $^ $(LD_FLAGS) -shared -o $@

clean:
	rm -f *.o *~ $(TARGET) $(TARGET).so $(TARGET).asm

help:
	@echo 'all builds so and exe. all is the default.'
	@echo 'clean runs clean.'
	@echo 'help runs help.'

