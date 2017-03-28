#define BITS_PER_BYTE 8
#define DOWN_THRESHOLD 50 
#define UP_THRESHOLD 200 
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Will be used for batching events
typedef enum EffectType {
	SINGLE_PIXEL,
	SINGLE_ROW,
	SINGLE_COLUMN,
	WHOLE_IMAGE
} EffectType;

typedef struct Effect {
	char* name;
	void (*function) (GdkPixbuf*, void**);
	EffectType type;
} Effect;

static GtkBuilder* builder = NULL; 
static GtkWidget* window = NULL;
static GtkWindow* about = NULL;
static GtkWidget* image = NULL;
static GdkPixbuf* image_buf = NULL;

void error(char* message) {
	fprintf(stderr, "%s\n", message);
	exit(-1);
}

unsigned min(unsigned a, unsigned b) {
	if (a < b) {return a;}
	return b;
}

unsigned max(unsigned a, unsigned b) {
	if (a > b) {return a;}
	return b;
}

guchar* addr(unsigned x, unsigned y, GdkPixbuf* image) {
	unsigned bytes_pp = gdk_pixbuf_get_n_channels(image) * (gdk_pixbuf_get_bits_per_sample(image) / 8);
	return gdk_pixbuf_get_pixels(image) + ((y * gdk_pixbuf_get_width(image)) + x) * bytes_pp;
}

unsigned pix_brightness(guchar* base) {
	unsigned res = 0;
	res += base[0];
	res += base[1];
	res += base[2];
	return res;
}

int cmp_brightness(const void* one, const void* two) {
	int diff = pix_brightness((guchar*) one) - pix_brightness((guchar*) two);
	if (diff <  0) return -1;
	else if (diff == 0) return 0;
	else return 1;
}

void sort_image(GdkPixbuf* image_buf) {
	printf("Sorting...\n");
	unsigned sorted_rows = 0;
	for (unsigned y = 0; y < gdk_pixbuf_get_height(image_buf); y++) {
		int stages = 0;
		size_t bp = 0;
		size_t x = gdk_pixbuf_get_width(image_buf) / 2;
		x += rand() % (x / 3);
		for (; x < gdk_pixbuf_get_width(image_buf); x++) {
			guchar* base = addr(x, y, image_buf);
			if (pix_brightness(base) > UP_THRESHOLD) {
				stages++;
				bp = x;
				break;
			}
		}
		for (; x < gdk_pixbuf_get_width(image_buf); x++) {
			guchar* base = addr(x, y, image_buf);
			if (pix_brightness(base) < DOWN_THRESHOLD) {
				stages++;
				break;
			}
		}

		x += rand() % (400);

		if (stages == 2) {
			sorted_rows++;
			qsort(
					addr(bp, y, image_buf), 
					x - bp, 
					(gdk_pixbuf_get_bits_per_sample(image_buf) / 8) * gdk_pixbuf_get_n_channels(image_buf), 
					cmp_brightness
				 );
		}
	}
	printf("Sorted\n");
}

void on_window_main_destroy() {
	printf("Goodbye!\n");
	gtk_main_quit();
}

void gtk_about_hide() {
	printf("hiding...\n");
	gtk_window_close(about);
}

void gtk_about_show() {
	if (about == NULL) {
		about = GTK_WINDOW(gtk_builder_get_object(builder, "about_glitcher"));
	}
	gtk_window_present(about);
}

void load_image(const char* in) {
	printf("Loading file: %s\n", in);
	GError* e = NULL;
	image_buf = gdk_pixbuf_new_from_file(in, &e);
	if (image_buf == NULL) {
		printf("Couldn't load image :(\n");
		exit(-1);
	}
	gtk_image_set_from_pixbuf((GtkImage*) image, image_buf);
	sort_image(image_buf);
}

void gtk_open_image() {
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkWidget* dialog = gtk_file_chooser_dialog_new(
			"Open File",
			(GtkWindow*) window,
			action,
			"_Cancel",
			GTK_RESPONSE_CANCEL,
			"_Open",
			GTK_RESPONSE_ACCEPT,
			NULL);
	gint res = gtk_dialog_run(GTK_DIALOG (dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		filename = gtk_file_chooser_get_filename (chooser);
		load_image(filename);
		g_free (filename);
	}
	gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
	gtk_init(&argc, &argv);
	builder = gtk_builder_new();
	gtk_builder_add_from_file (builder, "window_main.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
	image = GTK_WIDGET(gtk_builder_get_object(builder, "image_display"));
	gtk_builder_connect_signals(builder, NULL);
	gtk_window_present((GtkWindow*) window);
	gtk_main();
}
