all: sort.c
	gcc -o sort sort.c -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic
