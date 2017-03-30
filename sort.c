#define BITS_PER_BYTE 8
#define DOWN_THRESHOLD 50 
#define UP_THRESHOLD 200 
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Will be used for batching effects
typedef enum EffectType {
	SINGLE_PIXEL,
	SINGLE_ROW,
	SINGLE_COLUMN,
	WHOLE_IMAGE
} EffectType;

// Used to represent effects in the queue
typedef struct Effect {
	char* name;
	void (*function) (GdkPixbuf*, void**);
	EffectType type;
} Effect;

typedef struct Display {
	double scale;
	GdkPixbuf* image;
	GdkPixbuf* scaled_image;
} Display;

typedef struct Gui {
	GdkPixbuf* logo;
	GtkBuilder* builder; 
	GtkWidget* window;
	GtkWidget* add_effect;
	GtkWindow* about;
	GtkWidget* image;
} Gui;

static Display display;
static Gui gui;

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

void rescale_image() {
	int height = gdk_pixbuf_get_height(display.image);
	int width = gdk_pixbuf_get_width(display.image);
	display.scaled_image = gdk_pixbuf_scale_simple(
			display.image, 
			(int) (((double) width) * display.scale),
			(int) (((double) height) * display.scale),
			GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf((GtkImage*) gui.image, display.scaled_image);
}

void gtk_zoom_one() {
	display.scale = 1.0;
	rescale_image();
}

void gtk_zoom_out() {
	display.scale -= 0.1;
	rescale_image();
}

void gtk_zoom_in() {
	display.scale += 0.1;
	rescale_image();
}

void sort_image(GdkPixbuf* image_buf) {
	printf("Sorting...\n");
	unsigned sorted_rows = 0;
	int height = gdk_pixbuf_get_height(image_buf);
	int width = gdk_pixbuf_get_width(image_buf);
	for (unsigned y = 0; y < height; y++) {
		int stages = 0;
		size_t bp = 0;
		size_t x = width / 2;
		x += rand() % (x / 3);
		for (; x < width; x++) {
			guchar* base = addr(x, y, image_buf);
			if (pix_brightness(base) > UP_THRESHOLD) {
				stages++;
				bp = x;
				break;
			}
		}
		for (; x < width; x++) {
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

void gtk_about_close(GtkDialog *about_glitcher) {
	printf("hiding...\n");
	gtk_window_close(gui.about);
}

void gtk_about_show() {
	gtk_show_about_dialog((GtkWindow*) gui.window,
		"program-name", strings.name,
		"logo", gui.logo,
		"title", strings.about,
		"license-type", GTK_LICENSE_GPL_3_0,
		"authors", strings.authors,
		"version", strings.version,
		"website", strings.website,
		"comments", strings.comment,
		NULL
		);
}

void load_image(const char* in) {
	printf("Loading file: %s\n", in);
	GError* e = NULL;
	display.image = gdk_pixbuf_new_from_file(in, &e);
	if (display.image == NULL) {
		printf("Couldn't load image :(\n");
		exit(-1);
	}
	display.scaled_image = display.image;
	display.scale = 1.0;
	gtk_image_set_from_pixbuf((GtkImage*) gui.image, display.scaled_image);
}

void gtk_open_image() {
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkWidget* dialog = gtk_file_chooser_dialog_new(
			"Open File",
			(GtkWindow*) gui.window,
			action,
			"_Cancel",
			GTK_RESPONSE_CANCEL,
			"_Open",
			GTK_RESPONSE_ACCEPT,
			NULL);
	gint res = gtk_dialog_run(GTK_DIALOG(dialog));
	if (res == GTK_RESPONSE_ACCEPT) {
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		filename = gtk_file_chooser_get_filename (chooser);
		load_image(filename);
		g_free (filename);
	}
	gtk_widget_destroy(dialog);
}

void gtk_add_effect() {
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
			"Message", GTK_WINDOW(gui.window), flags,
			("Cancel"), GTK_RESPONSE_REJECT,
			("OK"), GTK_RESPONSE_ACCEPT,
			NULL);
	GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget* label = gtk_label_new("Hello, World!");

	// Ensure that the dialog box is destroyed when the user responds
	g_signal_connect_swapped (dialog, "response",
			G_CALLBACK(gtk_widget_destroy), dialog);

	// Add the label, and show everything we’ve added
	gtk_container_add(GTK_CONTAINER(content_area), label);
	
	gtk_widget_show_all (dialog);
}

void gtk_effect_select() {
	printf("effect changed\n");
}

void gtk_close_effect_chooser() {

}

int main(int argc, char *argv[]) {
	strings.comment = "Made with ♥ by Owen";
	strings.logo_path = "logo.png";
	strings.name = "Glitcher of Gorts";
	strings.version = "1.0";
	strings.website = "https://owen.cafe/glitcher/";
	const char* authors[] = {
		"Owen Shepherd",
		NULL
	};
	strings.authors = authors;
	GError* e = NULL;
	gui.logo = gdk_pixbuf_new_from_file(strings.logo_path, &e);
	char about[24];
	strings.about = about;
	sprintf(strings.about, "About %s", strings.name);
	gtk_init(&argc, &argv);
	gui.builder = gtk_builder_new();
	gtk_builder_add_from_file (gui.builder, "window_main.glade", NULL);
	gui.window = GTK_WIDGET(gtk_builder_get_object(gui.builder, "window_main"));
	gui.image = GTK_WIDGET(gtk_builder_get_object(gui.builder, "image_display"));
	gtk_builder_connect_signals(gui.builder, NULL);
	gtk_window_present((GtkWindow*) gui.window);
	gtk_main();
}
