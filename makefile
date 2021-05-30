cc = gcc
cflags = -Wall

myShell : myshell.c
	$(cc) $(cflags) -o myShell $<

clean :
	rm myShell