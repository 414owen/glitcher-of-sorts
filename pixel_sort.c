#include "common.h"
#include "pixel_sort.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void* new_sort_settings_hor(ImageDeets* deets) {
	SortSettings* res = malloc(sizeof(SortSettings));
	res->deets = deets;
	res->start_d = 0.0;
	res->up_threshold_d = 50.0;
	res->down_threshold_d = 200.0;

	// TODO figure this bit out, maybe just make others a separate sort?
	res->mode = BRIGHTNESS_SORT;

	res->pixs = deets->width;
	res->plus_random_start_d = 0.0;
	res->minus_random_end_d = 0.0;
	res->edge_is_threshold = true;
	res->comparator = false;
	res->cmp_func = cmp_brightness_gt;
	res->bytes_pp = deets->bytes_pp;
	return res;
}

bool validate_sort_settings(void* settings_v) {
	SortSettings* settings = (SortSettings*) settings_v;
	settings->start = settings->start_d;
	settings->plus_random_start = settings->plus_random_start_d;
	settings->minus_random_end = settings->minus_random_end_d;
	settings->random_max = settings->random_max_d;
	settings->up_threshold = settings->up_threshold_d;
	settings->down_threshold = settings->down_threshold_d;
	settings->cmp_func = settings->comparator ? cmp_brightness_lt : cmp_brightness_gt;
	return true;
}

void* copy_sort_settings(void* settings_v) {
	SortSettings* res = malloc(sizeof(SortSettings));
	memcpy(res, settings_v, sizeof(SortSettings));
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
	SettingType range_type = RANGE_SETTING;
	SettingType bool_type  = BOOLEAN_SETTING;
	char* labels[] = {
		"Starting Point",
		"Edge is Threshold",
		"Start Threshold",
		"End Threshold",
		"Plus Start Random",
		"Minus End Random",
		"Decreasing Sort"
	};
	double zero = 0.0;
	double width_d = (double) settings->deets->width;
	double max_brightness = 765.0;
	void* args[] = {

		&range_type,
		labels[0],
		&settings->start_d,
		&zero,
		&width_d,

		&range_type,
		labels[2],
		&settings->up_threshold_d,
		&zero,
		&max_brightness,
		
		&range_type,
		labels[3],
		&settings->down_threshold_d,
		&zero,
		&max_brightness,

		&range_type,
		labels[4],
		&settings->plus_random_start_d,
		&zero,
		&max_brightness,

		&range_type,
		labels[5],
		&settings->minus_random_end_d,
		&zero,
		&max_brightness,

		&bool_type,
		labels[1],
		&settings->edge_is_threshold,
		
		&bool_type,
		labels[6],
		&settings->comparator,
		
		NULL
	};

	return generate_settings_ui(args);
}

void sort_horizontal(guchar* data, void* settings_v) {
	SortSettings* settings = (SortSettings*) settings_v;
	int stages = 0;
	size_t bp = 0;
	size_t x = settings->start;
	x += rand() % (settings->plus_random_start);
	for (; x < settings->pixs; x++) {
		guchar* base = data + x * settings->bytes_pp;
		if (pix_brightness(base) > settings->up_threshold) {
			stages++;
			bp = x;
			break;
		}
	}
	for (; x < settings->pixs; x++) {
		guchar* base = data + x * settings->bytes_pp;
		if (pix_brightness(base) < settings->down_threshold) {
			stages++;
			break;
		}
	}
	x -= rand() % (settings->minus_random_end);
	if (stages == 2) {
		qsort(
				data + bp * settings->bytes_pp,
				x - bp, 
				settings->bytes_pp,
				settings->cmp_func
			 );
	}
}
