#include "common.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <stdarg.h>

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



/*
 * Takes a SettingType, a pointer to its data, then its parameters...
 */
GtkWidget* generate_settings_ui(void** args) {
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	int ind = 0;
	while (args[ind] != NULL) {
		void* arg_t = args[ind++];
		char* label_text = (char*) args[ind++];
		GtkWidget* label = gtk_label_new(label_text);
		gtk_container_add(GTK_CONTAINER(box), label);
		switch(*((SettingType*) arg_t)) {
			case BOOLEAN_SETTING: {
				printf("Bool setting\n");
				break;
			}
			case UNSIGNED_SETTING: {
				double initial   = (double) *((unsigned*) args[ind++]);
				double min_scale = (double) *((unsigned*) args[ind++]);
				double max_scale = (double) *((unsigned*) args[ind++]);
				double step = (double) (max_scale - min_scale) / 20.0;
				GtkAdjustment* adjustment = gtk_adjustment_new(
					initial,
                    min_scale,
                    max_scale,
                    step,
                    step,
                    step
				);
				GtkWidget* scale = gtk_scale_new(
					GTK_ORIENTATION_HORIZONTAL,
					adjustment
				);
				gtk_container_add(GTK_CONTAINER(box), scale);
				printf("Int setting\n");
				break;
			}
			default: {
				printf("Unknown argument\n");
				break;
			}
		}
	}
	return box;
}
