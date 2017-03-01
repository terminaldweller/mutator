
##################################VARS#################################
CC=clang
CC_FLAGS=
LD_FLAGS=
TARGET=mutatord
##################################RULES################################
.DEFAULT: all

.PHONY: all clean help

all: $(TARGET)

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

TARGET:
	$(CC) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(TARGET)

help:
	@echo 'all builds the target. all is the default.'
	@echo 'clean runs clean.'
	@echo 'help runs help.'

