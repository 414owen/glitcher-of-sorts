all: sort.c
	gcc -std=c11 -o sort sort.c common.c pixel_sort.c -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic
