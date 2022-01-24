/*
 * Copyright 2022 Steven Benner
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "interface.h"

#include <gtk/gtk.h>

#include "database.h"

struct main_window_gui_t main_window_gui;

static void create_header_bar(void)
{
	GtkWidget *header_bar, *menu_image;

	header_bar = gtk_header_bar_new();
	gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "PacFinder");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

	main_window_gui.menu_button = gtk_menu_button_new();
	menu_image = gtk_image_new_from_icon_name("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(main_window_gui.menu_button), menu_image);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), main_window_gui.menu_button);

	gtk_window_set_titlebar(GTK_WINDOW(main_window_gui.window), header_bar);
}

static GtkWidget *create_repo_tree(void)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *scrolled_window;

	main_window_gui.repo_tree_store = gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_POINTER);
	main_window_gui.repo_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(main_window_gui.repo_tree_store));

	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, "Repositories");
	gtk_tree_view_append_column(GTK_TREE_VIEW(main_window_gui.repo_treeview), column);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "stock-id", 0, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), main_window_gui.repo_treeview);
	gtk_widget_set_size_request(scrolled_window, 200, -1);

	return scrolled_window;
}

static void reason_cell_data_fn(GtkTreeViewColumn *column, GtkCellRenderer *renderer,
                                GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
	static const gchar *reason_map[] = {
		[PKG_REASON_NOT_INSTALLED] = "",
		[PKG_REASON_EXPLICIT] = "Explicit",
		[PKG_REASON_DEPEND] = "Depend",
		[PKG_REASON_OPTIONAL] = "Optional"
	};

	install_reason_t reason;
	gtk_tree_model_get(model, iter, 2, &reason, -1);
	g_object_set(renderer, "text", reason_map[reason], NULL);
}

static GtkWidget *create_package_list(void)
{
	const gint column_count = 4;
	const gchar *column_titles[] = { "Name", "Version", "Reason", "Repository" };

	GtkWidget *scrolled_window;
	gint i;

	main_window_gui.package_list_store = gtk_list_store_new(
		column_count + 1, /* column_count+1 for the non-visible pointer column */
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_INT,
		G_TYPE_STRING,
		G_TYPE_POINTER
	);

	main_window_gui.package_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(main_window_gui.package_list_store));

	for (i = 0; i < column_count; i++) {
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(
			column_titles[i],
			renderer,
			"text",
			i,
			NULL
		);

		if (i == 0) {
			g_object_set(renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
		}

		if (i == 2) {
			gtk_tree_view_column_set_cell_data_func(column, renderer, reason_cell_data_fn, NULL, NULL);
		}

		gtk_tree_view_column_set_fixed_width(column, 150);
		gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_resizable(column, TRUE);
		gtk_tree_view_column_set_sort_column_id(column, i);
		gtk_tree_view_append_column(GTK_TREE_VIEW(main_window_gui.package_treeview), column);
	}

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), main_window_gui.package_treeview);
	gtk_widget_set_size_request(scrolled_window, 400, 200);

	return scrolled_window;
}

static GtkWidget *create_package_overview(void)
{
	GtkWidget *aside_hbox, *middle_vbox, *right_vbox, *hbox, *scrolled_window;

	main_window_gui.details_overview.left_label = gtk_label_new(NULL);
	main_window_gui.details_overview.middle_label = gtk_label_new(NULL);
	main_window_gui.details_overview.right_label = gtk_label_new(NULL);

	aside_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(aside_hbox), main_window_gui.details_overview.left_label, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(aside_hbox), main_window_gui.details_overview.middle_label, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(aside_hbox), main_window_gui.details_overview.right_label, TRUE, FALSE, 0);

	main_window_gui.details_overview.heading_label = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(main_window_gui.details_overview.heading_label), GTK_JUSTIFY_LEFT);
	gtk_widget_set_halign(GTK_WIDGET(main_window_gui.details_overview.heading_label), GTK_ALIGN_START);
	gtk_label_set_xalign(GTK_LABEL(main_window_gui.details_overview.heading_label), 0);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.heading_label), 20);

	main_window_gui.details_overview.desc_label = gtk_label_new(NULL);
	gtk_label_set_justify(GTK_LABEL(main_window_gui.details_overview.desc_label), GTK_JUSTIFY_LEFT);
	gtk_widget_set_halign(GTK_WIDGET(main_window_gui.details_overview.desc_label), GTK_ALIGN_START);
	gtk_label_set_xalign(GTK_LABEL(main_window_gui.details_overview.desc_label), 0);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.desc_label), 20);
	gtk_label_set_line_wrap(GTK_LABEL(main_window_gui.details_overview.desc_label), TRUE);

	middle_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(middle_vbox), main_window_gui.details_overview.heading_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(middle_vbox), main_window_gui.details_overview.desc_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(middle_vbox), aside_hbox, FALSE, FALSE, 0);

	main_window_gui.details_overview.required_by_label = gtk_label_new(NULL);
	gtk_label_set_xalign(GTK_LABEL(main_window_gui.details_overview.required_by_label), 0);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.required_by_label), 20);
	main_window_gui.details_overview.optional_for_label = gtk_label_new(NULL);
	gtk_label_set_xalign(GTK_LABEL(main_window_gui.details_overview.optional_for_label), 0);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.optional_for_label), 20);
	main_window_gui.details_overview.dependencies_label = gtk_label_new(NULL);
	gtk_label_set_xalign(GTK_LABEL(main_window_gui.details_overview.dependencies_label), 0);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.dependencies_label), 20);

	right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_margin_start(GTK_WIDGET(right_vbox), 20);
	gtk_widget_set_margin_end(GTK_WIDGET(right_vbox), 20);
	gtk_box_pack_start(GTK_BOX(right_vbox), main_window_gui.details_overview.required_by_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), main_window_gui.details_overview.optional_for_label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), main_window_gui.details_overview.dependencies_label, FALSE, FALSE, 0);

	main_window_gui.details_overview.status_image = gtk_image_new();
	gtk_widget_set_halign(GTK_WIDGET(main_window_gui.details_overview.status_image), GTK_ALIGN_END);
	gtk_widget_set_valign(GTK_WIDGET(main_window_gui.details_overview.status_image), GTK_ALIGN_START);
	gtk_widget_set_margin_end(GTK_WIDGET(main_window_gui.details_overview.status_image), 10);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_margin_top(GTK_WIDGET(hbox), 10);
	gtk_widget_set_margin_start(GTK_WIDGET(hbox), 10);
	gtk_widget_set_margin_end(GTK_WIDGET(hbox), 10);
	gtk_box_pack_start(GTK_BOX(hbox), main_window_gui.details_overview.status_image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), middle_vbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), right_vbox, FALSE, FALSE, 0);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), hbox);

	return scrolled_window;
}

static GtkWidget *create_package_details(void)
{
	const gint column_count = 2;

	GtkWidget *scrolled_window, *package_details_treeview;
	gint i;

	main_window_gui.package_details_list_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	package_details_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(main_window_gui.package_details_list_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(package_details_treeview), FALSE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(package_details_treeview)), GTK_SELECTION_NONE);

	for (i = 0; i < column_count; i++) {
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes("", renderer, "text", i, NULL);

		if (i == 0) {
			g_object_set(renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
		}

		gtk_tree_view_append_column(GTK_TREE_VIEW(package_details_treeview), column);
	}

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), package_details_treeview);

	return scrolled_window;
}

static GtkWidget *create_package_info(void)
{
	main_window_gui.details_notebook = GTK_NOTEBOOK(gtk_notebook_new());

	gtk_notebook_append_page_menu(
		main_window_gui.details_notebook,
		create_package_overview(),
		gtk_label_new("Overview"),
		NULL
	);

	gtk_notebook_append_page_menu(
		main_window_gui.details_notebook,
		create_package_details(),
		gtk_label_new("Details"),
		NULL
	);

	return GTK_WIDGET(main_window_gui.details_notebook);
}

static void create_user_interface(void)
{
	GtkWidget *vpaned, *hpaned;

	create_header_bar();

	vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	gtk_paned_set_position(GTK_PANED(vpaned), 400);
	gtk_paned_pack1(GTK_PANED(vpaned), create_package_list(), TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(vpaned), create_package_info(), TRUE, FALSE);

	hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_position(GTK_PANED(hpaned), 200);
	gtk_paned_pack1(GTK_PANED(hpaned), create_repo_tree(), TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(hpaned), vpaned, TRUE, FALSE);

	gtk_container_add(GTK_CONTAINER(main_window_gui.window), hpaned);
}

void create_app_window(GtkApplication *app)
{
	main_window_gui.window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(main_window_gui.window), "PacFinder");
	gtk_window_set_default_size(GTK_WINDOW(main_window_gui.window), 900, 700);
	create_user_interface();
	gtk_widget_show_all(main_window_gui.window);
}
