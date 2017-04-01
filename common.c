#include "common.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

unsigned pix_brightness(guchar* base) {
	unsigned res = 0;
	res += base[0];
	res += base[1];
	res += base[2];
	return res;
}

int cmp_brightness_gt(const void* one, const void* two) {
	return pix_brightness((guchar*) one) - pix_brightness((guchar*) two);
}

int cmp_brightness_lt(const void* one, const void* two) {
	return pix_brightness((guchar*) two) - pix_brightness((guchar*) one);
}

guchar* row_addr(unsigned y, ImageDeets* deets) {
	return deets->image + y * deets->width * deets->bytes_pp;
}

guchar* addr(unsigned x, unsigned y, ImageDeets* deets) {
	return deets->image + ((y * deets->width) + x) * deets->bytes_pp;
}

unsigned min(unsigned a, unsigned b) {
	return a < b ? a : b;
}

unsigned max(unsigned a, unsigned b) {
	return b < a ? b : a;
}

ImageDeets* get_image_deets(GdkPixbuf* image) {
	ImageDeets* deets = malloc(sizeof(ImageDeets));
	deets->width = gdk_pixbuf_get_width(image);
	deets->height = gdk_pixbuf_get_height(image);
	deets->channels = gdk_pixbuf_get_n_channels(image);
	deets->bytes_pp = (gdk_pixbuf_get_bits_per_sample(image) / 8) * deets->channels;
	deets->image = gdk_pixbuf_get_pixels(image);
	return deets;
}
