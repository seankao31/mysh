.PHONY: test

all: mysh

test: myshtest

clean:
	rm -f mysh myshtest

mysh: mysh.c
	gcc $^ -o $@ -g

myshtest: mysh.c
	gcc $^ -o $@ -g -Dtest
