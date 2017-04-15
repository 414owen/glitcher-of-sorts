compile_flags = -std=c11 -o sort sort.c common.c pixel_sort.c -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic
CC = gcc

prod: sort.c
	$(CC) $(compile_flags) -O2

debug: sort.c
	$(CC) $(compile_flags) -g -D DEBUG -D RED_LINE_TEST
