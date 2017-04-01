compile_flags = -std=c11 -g -o sort sort.c common.c pixel_sort.c -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

gcc: sort.c
	gcc $(compile_flags)

clang: sort.c
	clang $(compile_flags)
