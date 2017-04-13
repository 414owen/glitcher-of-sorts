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
	GdkPixbuf* image;
	guchar* pixels;
} ImageDeets;

// Will be used for pipelining of effects
typedef enum EffectType {
	NO_EFFECT,
	PIXEL_EFFECT,
	ROW_EFFECT,
	// Same as row effect, but is fed flipped image
	COLUMN_EFFECT,
	IMAGE_EFFECT
} EffectType;

// Used to represent effects in the queue
typedef struct Effect {

	// Defines type of effect, used for effect pipelining
	EffectType type;

	// Name of the effect
	char* name;

	// Effect function, takes a pointer to the data, the length of the data, 
	// and the settings
	void (*function) (guchar*, void*);

	// Takes details, returns the settings
	void* (*new_settings) (ImageDeets*);

	// In case we need a fancy structures
	void (*free_settings) (void*);

	// Takes settings, returns the settings editor
	GtkWidget* (*new_settings_dialog) (void*);

	// Ideally, the user will only be presented with options that
	// can't conflict... Ah well, maybe this will be necessary.
	// Stores an error in the second argument
	// This function can also be used for pre-processing the settings
	bool (*validate) (void*, char**);

	// Creates a copy of its settings
	void* (*copy_settings) (void*);
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
