#pragma once

#include <stdbool.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

typedef struct ImageDeets {
	unsigned width;
	unsigned height;
	unsigned channels;
	unsigned bytes_pp;
	guchar* image;
} ImageDeets;

unsigned pix_brightness(guchar* base);
int cmp_brightness_gt(const void* one, const void* two);
int cmp_brightness_lt(const void* one, const void* two);
guchar* row_addr(unsigned y, ImageDeets* deets);
guchar* addr(unsigned x, unsigned y, ImageDeets* deets);
unsigned min(unsigned a, unsigned b);
unsigned max(unsigned a, unsigned b);
