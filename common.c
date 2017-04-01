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

void range_changed(GtkRange* range, void* user_data) {
	double d = gtk_range_get_value(range);
	eprintf("changed: value: %f\n", d);
	eprintf("changed: user_data: %p\n", user_data);
	*((double*) user_data) = d;
}

/*
 * Takes a SettingType, its label, a pointer to its data, then its parameters...
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
				eprintf("Bool setting\n");
				break;
			}
			case RANGE_SETTING: {
				eprintf("Range setting\n");
				double* value_p  = (double*) args[ind++];
				double initial   = *value_p;
				eprintf("initial-gen: %f\n", initial);
				double min_scale = *((double*) args[ind++]);
				double max_scale = *((double*) args[ind++]);
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
				eprintf("right before: %p\n", value_p);
				g_signal_connect(scale, "value_changed", G_CALLBACK(range_changed), value_p);
				gtk_container_add(GTK_CONTAINER(box), scale);
				break;
			}
			default: {
				eprintf("Unknown argument\n");
				break;
			}
		}
	}
	return box;
}
