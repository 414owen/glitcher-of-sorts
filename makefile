compile_flags = -std=c11 -o sort sort.c common.c pixel_sort.c -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

debug: sort.c
	gcc $(compile_flags) -g -D DEBUG

all: sort.c
	gcc $(compile_flags)

