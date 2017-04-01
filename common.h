#pragma once

#ifdef DEBUG
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#else 
#define eprintf(...)
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>

typedef struct Strings {
	char* about;
	const char* comment;
	const char* logo_path;
	const char* name;
	const char* version;
	const char* website;
	const char** authors;
} Strings;
Strings strings;

typedef struct ImageDeets {
	unsigned width;
	unsigned height;
	unsigned channels;
	unsigned bytes_pp;
	guchar* image;
} ImageDeets;

// Will be used for pipelining of effects
typedef enum EffectType {
	PIXEL_EFFECT,
	ROW_EFFECT,
	COLUMN_EFFECT,
	IMAGE_EFFECT
} EffectType;

// Used to represent effects in the queue
typedef struct Effect {

	// Name of the effect
	char* name;

	// Effect function, takesa pointer to the data, the details, and the settings
	void (*function) (guchar*, ImageDeets*, void*);

	// Takes details, returns the settings
	void* (*new_settings_struct) (ImageDeets*);

	// In case we need a fancy structures
	void (*free_settings_struct) (void*);

	// Takes settings, returns the settings editor
	GtkWidget* (*new_settings_dialog) (void*);

	// Defines type of effect, used for effect pipelining
	EffectType type;

} Effect;

typedef enum SettingType {
	BOOLEAN_SETTING,
	RANGE_SETTING
} SettingType;

unsigned pix_brightness(guchar* base);
int cmp_brightness_gt(const void* one, const void* two);
int cmp_brightness_lt(const void* one, const void* two);
guchar* row_addr(unsigned y, ImageDeets* deets);
guchar* addr(unsigned x, unsigned y, ImageDeets* deets);
unsigned min(unsigned a, unsigned b);
unsigned max(unsigned a, unsigned b);
ImageDeets* get_image_deets(GdkPixbuf*);
// See definition for usage
GtkWidget* generate_settings_ui(void**);
