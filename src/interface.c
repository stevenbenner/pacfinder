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

GtkWidget *window;

static void create_header_bar(void)
{
	GtkWidget *header_bar;

	header_bar = gtk_header_bar_new();
	gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "PacFinder");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

	gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
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
	static gchar * const reason_map[] = {
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

static GtkWidget *create_package_details(void)
{
	const gint column_count = 2;
	const gchar *column_titles[] = { "Name", "Value" };

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

		gtk_tree_view_append_column(GTK_TREE_VIEW(package_details_treeview), column);
	}

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), package_details_treeview);

	return scrolled_window;
}

static GtkWidget *create_package_info(void)
{
	GtkNotebook *notebook;

	notebook = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(notebook), create_package_details(), gtk_label_new("Details"), NULL);

	return GTK_WIDGET(notebook);
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

	gtk_container_add(GTK_CONTAINER(window), hpaned);
}

void create_app_window(GtkApplication *app)
{
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "PacFinder");
	gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);
	create_user_interface();
	gtk_widget_show_all(window);
}
