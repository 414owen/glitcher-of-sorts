#define BITS_PER_BYTE 8
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include "pixel_sort.h"
#include "common.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int greyed_no_image_num = 1;
const int greyed_no_effect_selected_num = 2;
const int effect_num = 3;

Effect SORT_HORIZONTAL_EFFECT = {
	.name = "Horizontal Sort", 
	.function = pixel_sort, 
	.new_settings = new_sort_settings_hor,
	.new_settings_dialog = new_sort_dialog,
	.copy_settings = copy_sort_settings,
	.validate = validate_sort_settings,
	.type = ROW_EFFECT
};

Effect SORT_WHOLE_EFFECT = {
	.name = "Whole Image Sort", 
	.function = pixel_sort, 
	.new_settings = new_sort_settings_whole,
	.new_settings_dialog = new_sort_dialog,
	.copy_settings = copy_sort_settings,
	.validate = validate_sort_settings,
	.type = IMAGE_EFFECT
};

Effect SORT_VERTICAL_EFFECT = {
	.name = "Vertical Sort", 
	.function = pixel_sort, 
	.new_settings = new_sort_settings_ver,
	.new_settings_dialog = new_sort_dialog,
	.copy_settings = copy_sort_settings,
	.validate = validate_sort_settings,
	.type = COLUMN_EFFECT
};

// List store enum
enum {
	COL_NAME = 0,
	COL_EFFECT = 1,
	COL_SETTINGS = 2,
	NUM_COLS = 3
};

typedef struct SwappableSetting {
	GtkWidget* setting_ui;
	gint effect_ind;
	GtkContainer* parent;
} SwappableSetting;

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
	GtkBox* side_pane;
	GtkWidget* effect_list_view;
	GtkCellRenderer* text_renderer;
	GtkListStore* effect_list;
	GtkTreeIter effect_list_iter;
} Gui;

static Display display;
static Gui gui;
GtkWidget** greyed_no_image;
GtkWidget** greyed_no_effect_selected;
Effect** effects;
void** effect_settings;
SwappableSetting swappable_setting;
GdkPixbuf* base_image;

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
	GdkPixbuf* temp = gdk_pixbuf_new_from_file(in, &e);
	display.image = gdk_pixbuf_new_from_data(
				gdk_pixbuf_get_pixels(temp),
			 	gdk_pixbuf_get_colorspace(temp),
				gdk_pixbuf_get_has_alpha(temp),
				gdk_pixbuf_get_bits_per_sample(temp),
				gdk_pixbuf_get_width(temp),
				gdk_pixbuf_get_height(temp),
				gdk_pixbuf_get_rowstride(temp),
				NULL,
				temp);
	base_image = display.image;
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
	ImageDeets* deets = get_image_deets(display.image);
	printf("Image loaded. Width: %d. Height: %d.\n", deets->width, deets->height);
	for (int i = 0; i < effect_num; i++) {
		effect_settings[i] = (void*) effects[i]->new_settings(deets);
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

void effect_selected_dropdown(GtkComboBox *combo, GtkWidget* accept) {
	/* GtkTreeIter iter; */
	/* gtk_combo_box_get_active_iter(combo, &iter); */
	/* Effect* effect; */
	/* void* settings; */
	/* gtk_tree_model_get( */
	/*         GTK_TREE_MODEL(gui.effect_list), &iter, */
	/*         COL_EFFECT, &effect, */
	/*         COL_SETTINGS, &settings, */
	/*         -1); */
	gint curr = gtk_combo_box_get_active(combo);
	printf("%s effect selected\n", effects[curr]->name);
	swappable_setting.effect_ind = curr;
	gtk_widget_set_sensitive(accept, true);
	if (swappable_setting.setting_ui != NULL) {
		gtk_container_remove(GTK_CONTAINER(swappable_setting.parent), swappable_setting.setting_ui);
	}
	swappable_setting.setting_ui = effects[curr]->new_settings_dialog(effect_settings[curr]);
	gtk_container_add(GTK_CONTAINER(swappable_setting.parent), swappable_setting.setting_ui);
	gtk_widget_show_all(swappable_setting.setting_ui);
}

void effect_add_callback(GtkWidget* widget, gint id, gpointer user_data) {
	gtk_widget_destroy(widget);
	if (id == GTK_RESPONSE_ACCEPT) {
		gtk_list_store_append(GTK_LIST_STORE(gui.effect_list), &gui.effect_list_iter);
		gint curr = swappable_setting.effect_ind;
		void* settings_copy = effects[curr]->copy_settings(effect_settings[curr]);
		char* err = NULL;
		bool success = effects[curr]->validate(settings_copy, &err);
		if (!success) {
			fprintf(stderr, "Error, cannot add effect: %s\n", err);
		} else {
			printf("%s effect queued\n", effects[curr]->name);
			gtk_list_store_set(
					GTK_LIST_STORE(gui.effect_list), 
					&gui.effect_list_iter,
					COL_NAME, effects[curr]->name,
					COL_EFFECT, effects[curr],
					COL_SETTINGS, settings_copy,
					-1);
		}
	}
}

void gtk_add_effect() {
	swappable_setting.setting_ui = NULL;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
			"Effect Settings", GTK_WINDOW(gui.window), flags,
			("_Cancel"), GTK_RESPONSE_REJECT,
			("_OK"), GTK_RESPONSE_ACCEPT,
			NULL);

	GtkWidget* accept = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
	gtk_widget_set_sensitive(accept, false);

	// TODO figure out resizing
	gtk_window_set_resizable(GTK_WINDOW(dialog), true);
	GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_box_set_spacing(GTK_BOX(content_area), 10);
	swappable_setting.parent = GTK_CONTAINER(content_area);
	GtkWidget* combo = gtk_combo_box_text_new();
	gtk_window_set_modal(GTK_WINDOW(dialog), true);
	for (guchar i = 0; i < effect_num; i++) {
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, effects[i]->name);
	}
	g_signal_connect_after(combo, "changed", G_CALLBACK(effect_selected_dropdown), accept);

	// Ensure that the dialog box is destroyed when the user responds
	g_signal_connect(dialog, "response", G_CALLBACK(effect_add_callback), NULL);
	gtk_container_add(swappable_setting.parent, combo);
	gtk_widget_show_all(dialog);
}

void no_effect_selected_grey() {
	for (int i = 0; i < greyed_no_effect_selected_num; i++) {
		gtk_widget_set_sensitive(GTK_WIDGET(greyed_no_effect_selected[i]), false);
	}
}

typedef struct IntermediateEffect {
	EffectType type;
	ImageDeets* rotated;
	ImageDeets* normal;
} IntermediateEffect;

void apply_effect_mutate(EffectType type, void (*function) (guchar*, void*), ImageDeets* deets, void* settings) {
	guchar* pixels = deets->pixels;
	unsigned width = deets->width;
	unsigned height = deets->height;
	unsigned bytes_pp = deets->bytes_pp;
	switch (type) {
		case ROW_EFFECT:
			printf("Row effect debug: width: %d, bpp: %d\n", width, bytes_pp);
			for (unsigned i = 0; i < width * height * bytes_pp; i += deets->rowstride) {
				/* printf("%d, ", i); */
				function(pixels + i, settings);
			}
			/* function(pixels, settings) */
			printf("\n");
			break;
		case PIXEL_EFFECT:
			for (unsigned i = 0; i < width * height * bytes_pp; i += bytes_pp) {
				function(pixels + i, settings);
			}
			break;
		case IMAGE_EFFECT:
			function(pixels, settings);
			break;
		default:
			break;
	}
}

ImageDeets* apply_effect(Effect* effect, IntermediateEffect* previous, void* settings) {
	ImageDeets* deets;
	if (effect->type == COLUMN_EFFECT) {
		if (previous->type == COLUMN_EFFECT) {
			deets = previous->rotated;
		} else {
			printf("Normal: %d:%d\n", previous->normal->width, previous->normal->height);
			printf("Rotating image counter_clockwise\n");
			deets = get_image_deets(gdk_pixbuf_rotate_simple(previous->normal->image, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE));
			printf("Rotated: %d:%d\n", deets->width, deets->height);
		}
	} else {
		deets = get_image_deets(gdk_pixbuf_copy(previous->normal->image));
		previous->normal = deets;
	}
	apply_effect_mutate(
			effect->type == COLUMN_EFFECT ? ROW_EFFECT : effect->type, 
			effect->function, deets, settings);
	if (effect->type == COLUMN_EFFECT) {
		previous->rotated = deets;
		printf("Rotating image clockwise\n");
		deets = get_image_deets(gdk_pixbuf_rotate_simple(deets->image, GDK_PIXBUF_ROTATE_CLOCKWISE));
		printf("Rotated: %d:%d\n", deets->width, deets->height);
		previous->normal = deets;
	}
	return deets;
}

gboolean apply_effect_callback(GtkTreeModel *model,
		GtkTreePath *path,
		GtkTreeIter *iter,
		gpointer data) {
	IntermediateEffect* previous = (IntermediateEffect*) data;
	Effect* effect;
	void* settings;
	gtk_tree_model_get(
			model, iter,
			COL_EFFECT, &effect,
			COL_SETTINGS, &settings,
			-1);
	printf("Applying effect: %s\n", effect->name);
	apply_effect(effect, previous, settings);
	display.image = previous->normal->image;
	display.scaled_image = previous->normal->image;
	previous->type = effect->type;
	return false;
}

void gtk_sort() {
	printf("Sorting...\n");
	IntermediateEffect intermediate = {
		.type = NO_EFFECT,
		.normal = get_image_deets(base_image)
	};
	display.image = base_image;
	gtk_tree_model_foreach(GTK_TREE_MODEL(gui.effect_list), apply_effect_callback, &intermediate);
	printf("Sorted image\n");
	display.image = intermediate.normal->image;
	display.scaled_image = intermediate.normal->image;
	rescale_image();
}

int main(int argc, char *argv[]) {

	// Init Vocabulary
	const char* authors[] = {
		"Owen Shepherd",
		NULL
	};
	strings.about = malloc(24);
	strings.authors = authors;
	strings.comment = "Made with ♥ by Owen";
	strings.logo_path = "logo.png";
	strings.name = "Glitcher of Gorts";
	strings.version = "1.0";
	strings.website = "https://owen.cafe/glitcher/";
	sprintf(strings.about, "About %s", strings.name);

	// Init effects
	effects = malloc(sizeof(Effect*) * effect_num);
	effects[0] = &SORT_HORIZONTAL_EFFECT;
	effects[1] = &SORT_VERTICAL_EFFECT;
	effects[2] = &SORT_WHOLE_EFFECT;
	effect_settings = malloc(sizeof(void*) * effect_num);

	// Init GUI
	gtk_init(&argc, &argv);
	gui.builder = gtk_builder_new();
	gtk_builder_add_from_file (gui.builder, "window_main.glade", NULL);
	gui.window = GTK_WIDGET(gtk_builder_get_object(gui.builder, "window_main"));
	gui.image = GTK_WIDGET(gtk_builder_get_object(gui.builder, "image_display"));
	gtk_builder_connect_signals(gui.builder, NULL);
	GError* e = NULL;
	gui.text_renderer = gtk_cell_renderer_text_new();
	gui.logo = gdk_pixbuf_new_from_file(strings.logo_path, &e);
	gui.effect_list = GTK_LIST_STORE(gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER));
	gui.effect_list_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(gui.effect_list));
	gtk_tree_view_insert_column_with_attributes(
			GTK_TREE_VIEW(gui.effect_list_view),
			-1, "Effect Queue",  
			gui.text_renderer,
			"text", COL_NAME,
			NULL);
	gtk_widget_show_all(gui.effect_list_view);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(gui.effect_list_view), true);
	gui.side_pane = GTK_BOX(gtk_builder_get_object(gui.builder, "side_pane"));
	gtk_widget_show(gui.effect_list_view);
	GtkWidget* sort_button = gtk_button_new_with_label("Sort");
	gtk_box_pack_end(gui.side_pane, sort_button, false, false, 0);
	gtk_box_pack_end(gui.side_pane, gui.effect_list_view, true, true, 10);
	g_signal_connect(sort_button, "clicked", G_CALLBACK(gtk_sort), NULL);
	gtk_widget_show(sort_button);

	// Init greyed
	greyed_no_image = malloc(sizeof(GtkWidget*) * greyed_no_image_num);
	greyed_no_image[0] = GTK_WIDGET(gtk_builder_get_object(gui.builder, "add_effect_button"));
	for (int i = 0; i < greyed_no_image_num; i++) {
		gtk_widget_set_sensitive(GTK_WIDGET(greyed_no_image[i]), false);
	}

	// Malloc some settings
	effect_settings = malloc(sizeof(void*) * effect_num);

	greyed_no_effect_selected = malloc(sizeof(GtkWidget*) * greyed_no_image_num);
	greyed_no_effect_selected[0] = GTK_WIDGET(gtk_builder_get_object(gui.builder, "edit_effect_button"));
	greyed_no_effect_selected[1] = GTK_WIDGET(gtk_builder_get_object(gui.builder, "remove_effect_button"));
	no_effect_selected_grey();

	gtk_window_present((GtkWindow*) gui.window);
	gtk_main();
}
