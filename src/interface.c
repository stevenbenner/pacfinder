/* interface.c - PacFinder graphical user interface layout and initialization
 *
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

/* build config */
#include "config.h"

/* file header */
#include "interface.h"

/* system libraries */
#include <glib/gi18n.h>
#include <gtk/gtk.h>

/* pacfinder */
#include "database.h"

struct main_window_gui_t main_window_gui;

static GtkWidget *create_header_bar(void)
{
	GtkWidget *header_bar, *menu_image;

	header_bar = gtk_header_bar_new();
	/* l10n: main window title */
	gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), _("PacFinder"));
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);

	main_window_gui.search_entry = gtk_search_entry_new();
	gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), main_window_gui.search_entry);

	main_window_gui.menu_button = gtk_menu_button_new();
	menu_image = gtk_image_new_from_icon_name("open-menu-symbolic", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image(GTK_BUTTON(main_window_gui.menu_button), menu_image);
	gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), main_window_gui.menu_button);

	return header_bar;
}

static GtkWidget *create_repo_tree(void)
{
	GtkTreeViewColumn *column;
	GtkCellRenderer *renderer;
	GtkWidget *scrolled_window;

	main_window_gui.repo_tree_store = gtk_tree_store_new(
		FILTERS_NUM_COLS,
		G_TYPE_STRING,  /* stock image */
		G_TYPE_STRING,  /* item name */
		G_TYPE_INT,     /* filters */
		G_TYPE_POINTER, /* database */
		G_TYPE_POINTER  /* group */
	);
	main_window_gui.repo_treeview = GTK_TREE_VIEW(
		gtk_tree_view_new_with_model(GTK_TREE_MODEL(main_window_gui.repo_tree_store))
	);

	column = gtk_tree_view_column_new();
	/* l10n: filter list tree view heading */
	gtk_tree_view_column_set_title(column, _("Repositories"));
	gtk_tree_view_append_column(main_window_gui.repo_treeview, column);

	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_tree_view_column_pack_start(column, renderer, FALSE);
	gtk_tree_view_column_set_attributes(column, renderer, "stock-id", FILTERS_COL_ICON, NULL);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", FILTERS_COL_TITLE, NULL);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(main_window_gui.repo_treeview));

	return scrolled_window;
}

static void reason_cell_data_fn(GtkTreeViewColumn *column, GtkCellRenderer *renderer,
                                GtkTreeModel *model, GtkTreeIter *iter, gpointer user_data)
{
	static const gchar *reason_map[] = {
		[PKG_REASON_NOT_INSTALLED] = "",
		/* l10n: package install reasons - shown in package list */
		[PKG_REASON_EXPLICIT] = N_("Explicit"),
		[PKG_REASON_DEPEND] = N_("Depend"),
		[PKG_REASON_OPTIONAL] = N_("Optional"),
		[PKG_REASON_ORPHAN] = N_("Orphan")
	};

	install_reason_t reason;

	gtk_tree_model_get(model, iter, PACKAGES_COL_STATUS, &reason, -1);

	/* only set localized string for installed packages to
	 * prevent sending an empty string to gettext */
	if (reason == PKG_REASON_NOT_INSTALLED) {
		g_object_set(renderer, "text", "", NULL);
	} else {
		g_object_set(renderer, "text", _(reason_map[reason]), NULL);
	}
}

static GtkWidget *create_package_list(void)
{
	/* l10n: package list column names */
	const gchar *column_titles[] = { N_("Name"), N_("Version"), N_("Reason"), N_("Repository") };

	GtkWidget *scrolled_window;
	gint i;

	main_window_gui.package_list_store = gtk_list_store_new(
		PACKAGES_NUM_COLS,
		G_TYPE_STRING, /* name */
		G_TYPE_STRING, /* version */
		G_TYPE_INT,    /* reason */
		G_TYPE_STRING, /* repository */
		G_TYPE_POINTER /* alpm_pkg_t */
	);

	main_window_gui.package_treeview = GTK_TREE_VIEW(
		gtk_tree_view_new_with_model(GTK_TREE_MODEL(main_window_gui.package_list_store))
	);

	for (i = 0; i < PACKAGES_NUM_COLS - 1; i++) { /* COLS-1 for the non-visible pointer column */
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		renderer = gtk_cell_renderer_text_new();
		column = gtk_tree_view_column_new_with_attributes(
			_(column_titles[i]),
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

		gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_resizable(column, TRUE);
		gtk_tree_view_append_column(main_window_gui.package_treeview, column);
	}

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(main_window_gui.package_treeview));

	return scrolled_window;
}

static GtkWidget *create_package_overview(void)
{
	GtkWidget *aside_hbox, *middle_vbox, *required_by_heading, *optional_for_heading,
	          *dependencies_heading, *right_vbox, *hbox, *scrolled_window;

	main_window_gui.details_overview.left_label = GTK_LABEL(gtk_label_new(NULL));
	main_window_gui.details_overview.middle_label = GTK_LABEL(gtk_label_new(NULL));
	main_window_gui.details_overview.right_label = GTK_LABEL(gtk_label_new(NULL));

	aside_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(aside_hbox), GTK_WIDGET(main_window_gui.details_overview.left_label), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(aside_hbox), GTK_WIDGET(main_window_gui.details_overview.middle_label), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(aside_hbox), GTK_WIDGET(main_window_gui.details_overview.right_label), TRUE, FALSE, 0);

	main_window_gui.details_overview.heading_label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_justify(main_window_gui.details_overview.heading_label, GTK_JUSTIFY_LEFT);
	gtk_label_set_xalign(main_window_gui.details_overview.heading_label, 0);
	gtk_widget_set_halign(GTK_WIDGET(main_window_gui.details_overview.heading_label), GTK_ALIGN_START);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.heading_label), 20);

	main_window_gui.details_overview.desc_label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_justify(main_window_gui.details_overview.desc_label, GTK_JUSTIFY_LEFT);
	gtk_label_set_xalign(main_window_gui.details_overview.desc_label, 0);
	gtk_label_set_line_wrap(main_window_gui.details_overview.desc_label, TRUE);
	gtk_widget_set_halign(GTK_WIDGET(main_window_gui.details_overview.desc_label), GTK_ALIGN_START);

	main_window_gui.details_overview.link_label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_justify(main_window_gui.details_overview.link_label, GTK_JUSTIFY_LEFT);
	gtk_label_set_xalign(main_window_gui.details_overview.link_label, 0);
	gtk_label_set_line_wrap(main_window_gui.details_overview.link_label, TRUE);
	gtk_widget_set_halign(GTK_WIDGET(main_window_gui.details_overview.link_label), GTK_ALIGN_START);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.link_label), 20);

	middle_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(middle_vbox), GTK_WIDGET(main_window_gui.details_overview.heading_label), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(middle_vbox), GTK_WIDGET(main_window_gui.details_overview.desc_label), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(middle_vbox), GTK_WIDGET(main_window_gui.details_overview.link_label), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(middle_vbox), aside_hbox, FALSE, FALSE, 0);

	required_by_heading = gtk_label_new(NULL);
	gtk_label_set_xalign(GTK_LABEL(required_by_heading), 0);
	/* l10n: package overview tab right column labels */
	gtk_label_set_markup(GTK_LABEL(required_by_heading), _("<b><u>Required by:</u></b>"));
	main_window_gui.details_overview.required_by_label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_xalign(main_window_gui.details_overview.required_by_label, 0);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.required_by_label), 20);
	optional_for_heading = gtk_label_new(NULL);
	gtk_label_set_xalign(GTK_LABEL(optional_for_heading), 0);
	gtk_label_set_markup(GTK_LABEL(optional_for_heading), _("<b><u>Optional for:</u></b>"));
	main_window_gui.details_overview.optional_for_label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_xalign(main_window_gui.details_overview.optional_for_label, 0);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.optional_for_label), 20);
	dependencies_heading = gtk_label_new(NULL);
	gtk_label_set_xalign(GTK_LABEL(dependencies_heading), 0);
	gtk_label_set_markup(GTK_LABEL(dependencies_heading), _("<b><u>Dependencies:</u></b>"));
	main_window_gui.details_overview.dependencies_label = GTK_LABEL(gtk_label_new(NULL));
	gtk_label_set_xalign(main_window_gui.details_overview.dependencies_label, 0);
	gtk_widget_set_margin_bottom(GTK_WIDGET(main_window_gui.details_overview.dependencies_label), 20);

	right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_widget_set_margin_start(right_vbox, 20);
	gtk_widget_set_margin_end(right_vbox, 20);
	gtk_box_pack_start(GTK_BOX(right_vbox), required_by_heading, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), GTK_WIDGET(main_window_gui.details_overview.required_by_label), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), optional_for_heading, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), GTK_WIDGET(main_window_gui.details_overview.optional_for_label), FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), dependencies_heading, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), GTK_WIDGET(main_window_gui.details_overview.dependencies_label), FALSE, FALSE, 0);

	main_window_gui.details_overview.status_image = gtk_image_new();
	gtk_widget_set_halign(main_window_gui.details_overview.status_image, GTK_ALIGN_END);
	gtk_widget_set_valign(main_window_gui.details_overview.status_image, GTK_ALIGN_START);
	gtk_widget_set_margin_end(main_window_gui.details_overview.status_image, 10);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_margin_top(hbox, 10);
	gtk_widget_set_margin_start(hbox, 10);
	gtk_widget_set_margin_end(hbox, 10);
	gtk_box_pack_start(GTK_BOX(hbox), main_window_gui.details_overview.status_image, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), middle_vbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), right_vbox, FALSE, FALSE, 0);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), hbox);

	return scrolled_window;
}

static GtkWidget *create_package_dependencies(void)
{
	GtkWidget *requires_label, *optional_label, *grid, *scrolled_window;

	main_window_gui.package_details_deps_box = GTK_FLOW_BOX(gtk_flow_box_new());
	gtk_flow_box_set_selection_mode(main_window_gui.package_details_deps_box, GTK_SELECTION_NONE);
	gtk_widget_set_valign(GTK_WIDGET(main_window_gui.package_details_deps_box), GTK_ALIGN_START);
	gtk_widget_set_hexpand(GTK_WIDGET(main_window_gui.package_details_deps_box), TRUE);

	/* l10n: labels in package dependencies tab */
	requires_label = gtk_label_new(_("Requires:"));
	gtk_widget_set_valign(requires_label, GTK_ALIGN_START);
	gtk_widget_set_margin_top(requires_label, 5);
	gtk_widget_set_margin_start(requires_label, 5);
	gtk_widget_set_margin_end(requires_label, 5);

	optional_label = gtk_label_new(_("Optional:"));
	gtk_widget_set_valign(optional_label, GTK_ALIGN_START);
	gtk_widget_set_margin_top(optional_label, 5);
	gtk_widget_set_margin_start(optional_label, 5);
	gtk_widget_set_margin_end(optional_label, 5);

	main_window_gui.package_details_opts_grid = GTK_GRID(gtk_grid_new());
	gtk_grid_set_column_spacing(main_window_gui.package_details_opts_grid, 4);

	grid = gtk_grid_new();
	gtk_grid_attach(GTK_GRID(grid), requires_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(main_window_gui.package_details_deps_box), 1, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), optional_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(main_window_gui.package_details_opts_grid), 1, 1, 1, 1);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), grid);

	return scrolled_window;
}

static GtkWidget *create_package_dependents(void)
{
	GtkWidget *requires_label, *grid, *scrolled_window;

	main_window_gui.package_details_depsfor_box = GTK_FLOW_BOX(gtk_flow_box_new());
	gtk_flow_box_set_selection_mode(main_window_gui.package_details_depsfor_box, GTK_SELECTION_NONE);
	gtk_widget_set_valign(GTK_WIDGET(main_window_gui.package_details_depsfor_box), GTK_ALIGN_START);
	gtk_widget_set_hexpand(GTK_WIDGET(main_window_gui.package_details_depsfor_box), TRUE);

	/* l10n: labels in package dependents tab */
	requires_label = gtk_label_new(_("Required By:"));
	gtk_widget_set_valign(requires_label, GTK_ALIGN_START);
	gtk_widget_set_margin_top(requires_label, 5);
	gtk_widget_set_margin_start(requires_label, 5);
	gtk_widget_set_margin_end(requires_label, 5);

	grid = gtk_grid_new();
	gtk_grid_attach(GTK_GRID(grid), requires_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(main_window_gui.package_details_depsfor_box), 1, 0, 1, 1);

	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), grid);

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
		/* l10n: package overview tab name */
		gtk_label_new(_("Overview")),
		NULL
	);

	gtk_notebook_append_page_menu(
		main_window_gui.details_notebook,
		create_package_dependencies(),
		/* l10n: package dependencies tab name */
		gtk_label_new(_("Dependencies")),
		NULL
	);

	gtk_notebook_append_page_menu(
		main_window_gui.details_notebook,
		create_package_dependents(),
		/* l10n: package dependents tab name */
		gtk_label_new(_("Dependents")),
		NULL
	);

	gtk_notebook_append_page_menu(
		main_window_gui.details_notebook,
		create_package_details(),
		/* l10n: package details tab name */
		gtk_label_new(_("Details")),
		NULL
	);

	return GTK_WIDGET(main_window_gui.details_notebook);
}

static void create_user_interface(GtkWindow *window)
{
	gtk_window_set_titlebar(window, create_header_bar());

	main_window_gui.vpaned = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_VERTICAL));
	gtk_paned_add1(main_window_gui.vpaned, create_package_list());
	gtk_paned_add2(main_window_gui.vpaned, create_package_info());

	main_window_gui.hpaned = GTK_PANED(gtk_paned_new(GTK_ORIENTATION_HORIZONTAL));
	gtk_paned_add1(main_window_gui.hpaned, create_repo_tree());
	gtk_paned_add2(main_window_gui.hpaned, GTK_WIDGET(main_window_gui.vpaned));

	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(main_window_gui.hpaned));
}

void create_app_window(GtkApplication *app)
{
	main_window_gui.window = GTK_WINDOW(gtk_application_window_new(app));
	gtk_window_set_title(main_window_gui.window, _("PacFinder"));
	create_user_interface(main_window_gui.window);
}
