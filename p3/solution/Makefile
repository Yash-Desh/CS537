CC = gcc # specifying the compiler
CFLAGS = -Wall -Wextra -Werror -pedantic -std=gnu18	 # specifying the arguments for compilation
LOGIN = yashd # specifying you login
SUBMITPATH = ~cs537-1/handin/$(LOGIN)# specifying the path where you handin your files

.PHONY: all
all: wsh wsh-dbg

wsh: wsh.c wsh.h
	$(CC) $(CFLAGS) -O2 $^ -o $@

wsh-dbg: wsh.c wsh.h
	$(CC) $(CFLAGS) -Og -ggdb $^ -o $@

submit:
	cp ../../p3 $(SUBMITPATH)

.PHONY:clean
clean:
	rm -f *.o