
##################################VARS#################################
CC=clang
CC_FLAGS=-fpic
LD_FLAGS= -l bfd
TARGET=main
##################################RULES################################
.DEFAULT:all

.PHONY:all clean help $(TARGET) $(TARGET).so test

all:$(TARGET) $(TARGET).so test

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

$(TARGET): $(TARGET).o
	$(CC) $^ $(LD_FLAGS) -o $@

$(TARGET).so: $(TARGET).o
	$(CC) $^ $(LD_FLAGS) -shared -o $@

test:
	$(MAKE) -C ./test

clean:
	rm -f *.o *~ $(TARGET) $(TARGET).so
	$(MAKE) -C ./test clean


help:
	@echo 'all builds so and exe. all is the default.'
	@echo 'clean runs clean.'
	@echo 'help runs help.'

