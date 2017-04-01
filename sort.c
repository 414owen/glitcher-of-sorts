#define BITS_PER_BYTE 8
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include "pixel_sort.h"
#include "common.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int effect_num = 1;

Effect SORT_HORIZONTAL_EFFECT = {
	.name = "Horizontal Sort", 
	.function = sort_horizontal, 
	.new_settings_struct = new_sort_settings_hor,
	.free_settings_struct = free,
	.new_settings_dialog = new_sort_dialog,
	.type = ROW_EFFECT
};
typedef struct SwappableSetting {
	void** settings;
	GtkWidget* setting_ui;
	GtkContainer* parent;
} SwappableSetting;

int greyed_no_image_num = 1;
GtkWidget** greyed_no_image;
int greyed_no_effect_selected_num = 2;
GtkWidget** greyed_no_effect_selected;
Effect** effects;
ImageDeets* image_deets;
SwappableSetting swappable_setting;

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
	for (int i = 0; i < greyed_no_image_num; i++) {
		gtk_widget_set_sensitive(GTK_WIDGET(greyed_no_image[i]), true);
	}
	image_deets = get_image_deets(display.image);
	for (guchar i = 0; i < effect_num; i++) {
		swappable_setting.settings[i] = effects[i]->new_settings_struct(image_deets);
	}
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
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}


void effect_selected_dropdown(GtkComboBox *combo, gpointer this_will_never_be_used) {
	gint curr = gtk_combo_box_get_entry_text_column(combo);
	if (swappable_setting.setting_ui != NULL) {
		gtk_container_remove(GTK_CONTAINER(swappable_setting.parent), swappable_setting.setting_ui);
	}
	swappable_setting.setting_ui = effects[curr]->new_settings_dialog(swappable_setting.settings[curr]);
	gtk_container_add(GTK_CONTAINER(swappable_setting.parent), swappable_setting.setting_ui);
	gtk_widget_show(swappable_setting.setting_ui);
	printf("%d\n", curr);
}

void gtk_add_effect() {
	swappable_setting.setting_ui = NULL;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
			"Message", GTK_WINDOW(gui.window), flags,
			("Cancel"), GTK_RESPONSE_REJECT,
			("OK"), GTK_RESPONSE_ACCEPT,
			NULL);

	GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	swappable_setting.parent = GTK_CONTAINER(content_area);
	GtkWidget* combo = gtk_combo_box_text_new();
	gtk_window_set_modal(GTK_WINDOW(dialog), true);
	for (guchar i = 0; i < effect_num; i++) {
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, effects[i]->name);
	}
	g_signal_connect(combo, "changed", G_CALLBACK(effect_selected_dropdown), NULL);

	// Ensure that the dialog box is destroyed when the user responds
	g_signal_connect_swapped(dialog, "response",
			G_CALLBACK(gtk_widget_destroy), dialog);
	gtk_container_add(swappable_setting.parent, combo);
	gtk_widget_show_all(dialog);
}

void no_effect_selected_grey() {
	for (int i = 0; i < greyed_no_effect_selected_num; i++) {
		gtk_widget_set_sensitive(GTK_WIDGET(greyed_no_effect_selected[i]), false);
	}
}

void gtk_effect_select() {
	printf("effect changed\n");
}

int main(int argc, char *argv[]) {

	// Init Vocabulary
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

	// Init effects
	effects = malloc(sizeof(Effect*) * effect_num);
	effects[0] = &SORT_HORIZONTAL_EFFECT;

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

	// Init greyed
	greyed_no_image = malloc(sizeof(GtkWidget*) * greyed_no_image_num);
	greyed_no_image[0] = GTK_WIDGET(gtk_builder_get_object(gui.builder, "add_effect_button"));
	for (int i = 0; i < greyed_no_image_num; i++) {
		gtk_widget_set_sensitive(GTK_WIDGET(greyed_no_image[i]), false);
	}

	// Malloc some settings
	swappable_setting.settings = malloc(sizeof(void*) * effect_num);

	greyed_no_effect_selected = malloc(sizeof(GtkWidget*) * greyed_no_image_num);
	greyed_no_effect_selected[0] = GTK_WIDGET(gtk_builder_get_object(gui.builder, "edit_effect_button"));
	greyed_no_effect_selected[1] = GTK_WIDGET(gtk_builder_get_object(gui.builder, "remove_effect_button"));
	no_effect_selected_grey();

	gtk_window_present((GtkWindow*) gui.window);
	gtk_main();
}
