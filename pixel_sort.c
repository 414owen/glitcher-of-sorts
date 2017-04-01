#include "common.h"
#include "pixel_sort.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdlib.h>

void* new_sort_settings_hor(ImageDeets* deets) {
	SortSettings* res = malloc(sizeof(SortSettings));
	res->deets = deets;
	res->start = 0.0;
	res->edge_is_threshold = true;
	res->up_threshold = 50;
	res->down_threshold = 200;
	res->mode = BRIGHTNESS_SORT;
	res->pixs = deets->width;
	res->plus_random_start = 0;
	res->random_max = deets->width;
	res->comparator = false;
	res->cmp_func = cmp_brightness_gt;
	return res;
}

void* new_sort_settings_ver(ImageDeets* deets) {
	SortSettings* res = new_sort_settings_hor(deets);
	res->pixs = deets->height;
	return res;
}

void* new_sort_settings_whole(ImageDeets* deets) {
	SortSettings* res = new_sort_settings_hor(deets);
	res->pixs = deets->height * deets->width;
	return res;
}

GtkWidget* new_sort_dialog(void* settings_v) {
	SortSettings* settings = (SortSettings*) settings_v;
	SettingType type = RANGE_SETTING;
	char* labels[] = {
		"Starting Point"
	};
	double zero = 0.0;
	double width_d = (double) settings->deets->width;
	*(&(settings->start)) = 3.75;
	void* args[] = {
		&type,
		labels[0],
		&settings->start,
		&zero,
		&width_d,
		NULL
	};

	/* GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); */
	/* GtkWidget* label = gtk_label_new("No settings for now :("); */
	/* printf("Returning label\n"); */
	/* return label; */
	return generate_settings_ui(args);
}

void sort_horizontal(guchar* data, ImageDeets* deets, void* settings_v) {
	SortSettings* settings = (SortSettings*) settings_v;
	int stages = 0;
	size_t bp = 0;
	size_t x = deets->width / 2;
	x += rand() % (x / 3);
	for (; x < settings->pixs; x++) {
		guchar* base = data + x * deets->bytes_pp;
		if (pix_brightness(base) > settings->up_threshold) {
			stages++;
			bp = x;
			break;
		}
	}
	for (; x < settings->pixs; x++) {
		guchar* base = data + x * deets->bytes_pp;
		if (pix_brightness(base) < settings->down_threshold) {
			stages++;
			break;
		}
	}
	x += rand() % (400);
	if (stages == 2) {
		qsort(
				data + bp * deets->bytes_pp,
				x - bp, 
				deets->bytes_pp,
				settings->cmp_func
			 );
	}
}
