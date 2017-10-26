
##################################VARS#################################
CC=clang
CC_FLAGS=
LD_FLAGS= -l bfd
TARGET=main
##################################RULES################################
.DEFAULT:all

.PHONY:all clean help $(TARGET)

all:$(TARGET)

.c.o:
	$(CC) $(CC_FLAGS) -c $< -o $@

$(TARGET): $(TARGET).o
	$(CC) $^ $(LD_FLAGS) -o $@

clean:
	rm -f *.o *~ $(TARGET)

help:
	@echo 'all builds the daemon, the server and the client. all is the default.'
	@echo 'mutatord builds the daemon with the server'
	@echo 'mutatorc builds the client'
	@echo 'mutators builds the standalone server'
	@echo 'clean runs clean.'
	@echo 'help runs help.'

