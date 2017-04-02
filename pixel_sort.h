#pragma once 

#include "common.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdbool.h>

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

void sort_horizontal(guchar*, void*);
void* new_sort_settings_hor();
void* new_sort_settings_ver();
void* new_sort_settings_whole();
GtkWidget* new_sort_dialog(void*);
void* copy_sort_settings(void* settings_v);
