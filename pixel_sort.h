#pragma once 

#include "common.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdbool.h>

GtkWidget* new_full_sort_dialog(void*);
GtkWidget* new_sort_dialog(void*);
bool validate_sort_settings(void*, char**);
bool validate_sort_settings_full(void*, char**);
void full_pixel_sort(guchar*, void*);
void pixel_sort(guchar*, void*);
void* copy_sort_settings(void*);
void* copy_full_sort_settings(void*);
void* new_sort_settings_hor();
void* new_sort_settings_hor_full(ImageDeets*);
void* new_sort_settings_ver();
void* new_sort_settings_ver_full(ImageDeets*);
