#include "common.h"
#include "pixel_sort.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum SortMode {
	BRIGHTNESS_SORT, 
	DARKNESS_SORT
} SortMode;

typedef struct SortSettings {
	ImageDeets* deets;
	unsigned start;
	unsigned plus_random_start;
	unsigned minus_random_end;
	unsigned random_max;
	unsigned up_threshold;
	unsigned down_threshold;
	double start_d;
	double plus_random_start_d;
	double minus_random_end_d;
	double random_max_d;
	double up_threshold_d;
	double down_threshold_d;
	bool comparator;
	bool edge_is_threshold;
	unsigned pixs;
	unsigned char bytes_pp;
	SortMode mode;
	int (*cmp_func) (const void*, const void*);
} SortSettings;

typedef struct FullSortSettings {
	ImageDeets* deets;
	bool comparator;
	unsigned pixs;
	unsigned char bytes_pp;
	SortMode mode;
	int (*cmp_func) (const void*, const void*);
} FullSortSettings;

void* new_sort_settings_hor(ImageDeets* deets) {
	SortSettings* res = malloc(sizeof(SortSettings));
	res->deets = deets;
	res->start_d = 0.0;
	res->up_threshold_d = 200.0;
	res->down_threshold_d = 20.0;

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

bool validate_sort_settings(void* settings_v, char** err) {
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

void* copy_full_sort_settings(void* settings_v) {
	SortSettings* res = malloc(sizeof(SortSettings));
	memcpy(res, settings_v, sizeof(SortSettings));
	return res;
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

void* new_sort_settings_hor_full(ImageDeets* deets) {
	FullSortSettings* res = malloc(sizeof(FullSortSettings));
	res->pixs = deets->width;
	res->deets = deets;
	res->comparator = false;
	res->pixs = deets->width;
	res->bytes_pp = deets->bytes_pp;
	res->mode = BRIGHTNESS_SORT;
	res->cmp_func = cmp_brightness_gt;
	return res;
}

void* new_sort_settings_ver_full(ImageDeets* deets) {
	FullSortSettings* res = new_sort_settings_hor_full(deets);
	res->pixs = deets->height;
	return res;
}

bool validate_sort_settings_full(void* settings_v, char** err) {
	FullSortSettings* settings = (FullSortSettings*) settings_v;
	settings->cmp_func = settings->comparator ? cmp_brightness_lt : cmp_brightness_gt;
	return true;
}

char* labels[] = {
	"Starting Point",
	"Edge is Threshold",
	"Start Threshold",
	"End Threshold",
	"Plus Start Random",
	"Minus End Random",
	"Decreasing Sort"
};

GtkWidget* new_full_sort_dialog(void* settings_v) {
	SortSettings* settings = (SortSettings*) settings_v;
	SettingType bool_type  = BOOLEAN_SETTING;
	void* args[] = {
		&bool_type,
		labels[6],
		&settings->comparator,
		NULL
	};
	return generate_settings_ui(args);
}

GtkWidget* new_sort_dialog(void* settings_v) {
	SortSettings* settings = (SortSettings*) settings_v;
	SettingType range_type = RANGE_SETTING;
	SettingType bool_type  = BOOLEAN_SETTING;
	double zero = 0.0;
	double width_d = (double) settings->deets->width; double max_brightness = 765.0;
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

void pixel_sort(guchar* data, void* settings_v) {
	SortSettings* settings = (SortSettings*) settings_v;
	int stages = 0;
	size_t bp = 0;
	size_t xs = settings->start;
	size_t xe = settings->pixs;
	if (settings->plus_random_start > 0) {
		xs += rand() % settings->plus_random_start;
	}
	if (settings->minus_random_end > 0) {
		xe -= rand() % settings->minus_random_end;
	}

	size_t x;
	for (x = xs; x < xe; x++) {
		guchar* base = data + x * settings->bytes_pp;
		if (pix_brightness(base) > settings->up_threshold) {
			stages++;
			bp = x;
			break;
		}
	}
	for (; x < xe; x++) {
		guchar* base = data + x * settings->bytes_pp;
		if (pix_brightness(base) < settings->down_threshold) {
			stages++;
			break;
		}
	}
	if (stages == 2) {
		qsort(
				data + bp * settings->bytes_pp,
				x - bp, 
				settings->bytes_pp,
				settings->cmp_func
			 );
	}

	// Red test
#ifdef RED_LINE_TEST
	for (int i = 0; i < 10; i++) {
		*(data + i * settings->bytes_pp) = 255;
		*(data + (i + 1) * settings->bytes_pp) = 0;
		*(data + (i + 2) * settings->bytes_pp) = 0;
	}
#endif
}

void full_pixel_sort(guchar* data, void* settings_v) {
	FullSortSettings* settings = (FullSortSettings*) settings_v;
	qsort(
			data,
			settings->pixs,
			settings->bytes_pp,
			settings->cmp_func
		 );
}
