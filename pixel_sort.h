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
	double start;
	double plus_random_start;
	double random_max;
	bool comparator;
	bool edge_is_threshold;
	unsigned pixs;
	unsigned up_threshold;
	unsigned down_threshold;
	SortMode mode;
	int (*cmp_func) (const void*, const void*);
} SortSettings;

void sort_horizontal(guchar*, ImageDeets*, void*);
void* new_sort_settings_hor();
void* new_sort_settings_ver();
void* new_sort_settings_whole();
GtkWidget* new_sort_dialog(void*);
