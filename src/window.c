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

#include "config.h"

#include "window.h"

#include <alpm.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "aboutdialog.h"
#include "database.h"
#include "interface.h"
#include "main.h"
#include "util.h"

/* package list filtering */
enum {
	HIDE_NONE = 0,
	HIDE_INSTALLED = (1 << 0),
	HIDE_UNINSTALLED = (1 << 1),
	HIDE_EXPLICIT = (1 << 2),
	HIDE_DEPEND = (1 << 3),
	HIDE_OPTION = (1 << 4),
	HIDE_NATIVE = (1 << 5),
	HIDE_FOREIGN = (1 << 6)
};

/* package list filters */
struct _package_filters {
	guint status_filter;
	alpm_db_t *db;
	alpm_group_t *group;
	gchar *search_string;
};
struct _package_filters package_filters;

static gulong pkg_selchange_handler_id;
static GtkTreeModel *package_list_model;

static void show_package_list(void)
{
	alpm_list_t *i;
	GtkTreeIter iter;

	for (i = get_all_packages(); i; i = alpm_list_next(i)) {
		alpm_pkg_t *pkg = NULL;

		pkg = i->data;

		gtk_list_store_append(main_window_gui.package_list_store, &iter);
		gtk_list_store_set(
			main_window_gui.package_list_store,
			&iter,
			PACKAGES_COL_NAME, alpm_pkg_get_name(pkg),
			PACKAGES_COL_VERSION, alpm_pkg_get_version(pkg),
			PACKAGES_COL_STATUS, get_pkg_status(pkg),
			PACAKGES_COL_REPO, alpm_db_get_name(alpm_pkg_get_db(pkg)),
			PACKAGES_COL_PKG, pkg,
			-1
		);
	}

	package_list_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(main_window_gui.package_list_store), NULL);
	gtk_tree_view_set_model(GTK_TREE_VIEW(main_window_gui.package_treeview), GTK_TREE_MODEL(package_list_model));
	gtk_tree_sortable_set_sort_column_id(
		GTK_TREE_SORTABLE(main_window_gui.package_list_store),
		PACKAGES_COL_NAME,
		GTK_SORT_ASCENDING
	);
}

static void show_package_overview(alpm_pkg_t *pkg)
{
	gchar *str;
	alpm_list_t *dep_list;
	size_t count;

	/* set icon */
	switch (get_pkg_status(pkg)) {
		case PKG_REASON_EXPLICIT:
			gtk_image_set_from_icon_name(
				GTK_IMAGE(main_window_gui.details_overview.status_image),
				"gtk-yes",
				GTK_ICON_SIZE_DIALOG
			);
			break;
		case PKG_REASON_DEPEND:
			gtk_image_set_from_icon_name(
				GTK_IMAGE(main_window_gui.details_overview.status_image),
				"gtk-leave-fullscreen",
				GTK_ICON_SIZE_DIALOG
			);
			break;
		case PKG_REASON_OPTIONAL:
			gtk_image_set_from_icon_name(
				GTK_IMAGE(main_window_gui.details_overview.status_image),
				"gtk-connect",
				GTK_ICON_SIZE_DIALOG
			);
			break;
		case PKG_REASON_NOT_INSTALLED:
			gtk_image_set_from_icon_name(
				GTK_IMAGE(main_window_gui.details_overview.status_image),
				"open-menu-symbolic",
				GTK_ICON_SIZE_DIALOG
			);
			break;
	}

	/* set labels */
	str = g_strdup_printf(
		"<span size=\"xx-large\">%s</span>\n"
		"<i>version %s</i>",
		alpm_pkg_get_name(pkg),
		alpm_pkg_get_version(pkg)
	);
	gtk_label_set_markup(GTK_LABEL(main_window_gui.details_overview.heading_label), str);
	g_free(str);

	str = g_strdup_printf(
		"%s\n"
		"<a href=\"%s\">%s</a>",
		alpm_pkg_get_desc(pkg),
		alpm_pkg_get_url(pkg),
		alpm_pkg_get_url(pkg)
	);
	gtk_label_set_markup(GTK_LABEL(main_window_gui.details_overview.desc_label), str);
	g_free(str);

	str = human_readable_size(alpm_pkg_get_isize(pkg));
	gtk_label_set_label(GTK_LABEL(main_window_gui.details_overview.left_label), str);
	g_free(str);
	gtk_label_set_label(GTK_LABEL(main_window_gui.details_overview.middle_label), alpm_pkg_get_arch(pkg));
	gtk_label_set_label(GTK_LABEL(main_window_gui.details_overview.right_label), alpm_db_get_name(alpm_pkg_get_db(pkg)));

	dep_list = alpm_pkg_compute_requiredby(pkg);
	count = alpm_list_count(dep_list);
	/* l10n: package dependency counts - %ld will be a number (zero or more) */
	str = g_strdup_printf(ngettext("%ld package", "%ld packages", count), count);
	gtk_label_set_markup(GTK_LABEL(main_window_gui.details_overview.required_by_label), str);
	g_free(str);
	alpm_list_free_inner(dep_list, g_free);
	alpm_list_free(dep_list);

	dep_list = alpm_pkg_compute_optionalfor(pkg);
	count = alpm_list_count(dep_list);
	str = g_strdup_printf(ngettext("%ld package", "%ld packages", count), count);
	gtk_label_set_markup(GTK_LABEL(main_window_gui.details_overview.optional_for_label), str);
	g_free(str);
	alpm_list_free_inner(dep_list, g_free);
	alpm_list_free(dep_list);

	dep_list = alpm_pkg_get_depends(pkg);
	count = alpm_list_count(dep_list);
	str = g_strdup_printf(ngettext("%ld package", "%ld packages", count), count);
	gtk_label_set_markup(GTK_LABEL(main_window_gui.details_overview.dependencies_label), str);
	g_free(str);
}

static void show_package_details(alpm_pkg_t *pkg)
{
	GtkTreeIter iter;

	/* empty list from any previously selected package */
	gtk_list_store_clear(main_window_gui.package_details_list_store);

	/* add detail rows */
	gtk_list_store_append(main_window_gui.package_details_list_store, &iter);
	/* l10n: package details tab row labels */
	gtk_list_store_set(main_window_gui.package_details_list_store, &iter, 0, _("Name:"), 1, alpm_pkg_get_name(pkg), -1);
	gtk_list_store_append(main_window_gui.package_details_list_store, &iter);
	gtk_list_store_set(main_window_gui.package_details_list_store, &iter, 0, _("Version:"), 1, alpm_pkg_get_version(pkg), -1);
	gtk_list_store_append(main_window_gui.package_details_list_store, &iter);
	gtk_list_store_set(main_window_gui.package_details_list_store, &iter, 0, _("Description:"), 1, alpm_pkg_get_desc(pkg), -1);
	gtk_list_store_append(main_window_gui.package_details_list_store, &iter);
	gtk_list_store_set(main_window_gui.package_details_list_store, &iter, 0, _("Architecture:"), 1, alpm_pkg_get_arch(pkg), -1);
	gtk_list_store_append(main_window_gui.package_details_list_store, &iter);
	gtk_list_store_set(main_window_gui.package_details_list_store, &iter, 0, _("URL:"), 1, alpm_pkg_get_url(pkg), -1);
	gtk_list_store_append(main_window_gui.package_details_list_store, &iter);
	gtk_list_store_set(main_window_gui.package_details_list_store, &iter, 0, _("Packager:"), 1, alpm_pkg_get_packager(pkg), -1);
}

static void show_package(alpm_pkg_t *pkg)
{
	if (pkg == NULL) {
		gtk_widget_hide(GTK_WIDGET(main_window_gui.details_notebook));
		return;
	}

	show_package_overview(pkg);
	show_package_details(pkg);

	gtk_widget_show(GTK_WIDGET(main_window_gui.details_notebook));
}

static void package_row_selected(GtkTreeSelection *selection, gpointer user_data)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	alpm_pkg_t *pkg;

	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get(model, &iter, PACKAGES_COL_PKG, &pkg, -1);
		show_package(pkg);
	}
}

static void populate_db_tree_view(void)
{
	GtkTreeIter toplevel, child;
	alpm_list_t *i;

	/* add standard filter lists */
	gtk_tree_store_append(main_window_gui.repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		main_window_gui.repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, "gtk-home",
		/* l10n: filter names shown in main filter list */
		FILTERS_COL_TITLE, _("All Packages"),
		FILTERS_COL_MASK, HIDE_NONE,
		-1
	);

	gtk_tree_store_append(main_window_gui.repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		main_window_gui.repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, "gtk-media-play",
		FILTERS_COL_TITLE, _("Installed"),
		FILTERS_COL_MASK, HIDE_UNINSTALLED,
		-1
	);

	gtk_tree_store_append(main_window_gui.repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		main_window_gui.repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, "gtk-yes",
		FILTERS_COL_TITLE, _("Explicit"),
		FILTERS_COL_MASK, HIDE_UNINSTALLED | HIDE_DEPEND | HIDE_OPTION,
		-1
	);

	gtk_tree_store_append(main_window_gui.repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		main_window_gui.repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, "gtk-leave-fullscreen",
		FILTERS_COL_TITLE, _("Dependency"),
		FILTERS_COL_MASK, HIDE_UNINSTALLED | HIDE_EXPLICIT | HIDE_OPTION,
		-1
	);

	gtk_tree_store_append(main_window_gui.repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		main_window_gui.repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, "gtk-connect",
		FILTERS_COL_TITLE, _("Optional"),
		FILTERS_COL_MASK, HIDE_UNINSTALLED | HIDE_EXPLICIT | HIDE_DEPEND,
		-1
	);

	/* add known databases */
	for (i = alpm_get_syncdbs(get_alpm_handle()); i; i = i->next) {
		alpm_db_t *db;
		alpm_list_t *group_list;

		db = i->data;

		gtk_tree_store_append(main_window_gui.repo_tree_store, &toplevel, NULL);
		gtk_tree_store_set(main_window_gui.repo_tree_store, &toplevel,
			FILTERS_COL_ICON, "gtk-directory",
			FILTERS_COL_TITLE, alpm_db_get_name(db),
			FILTERS_COL_MASK, HIDE_NONE,
			FILTERS_COL_DB, db,
			-1
		);

		/* add any groups found for this database */
		group_list = alpm_db_get_groupcache(db);
		group_list = alpm_list_msort(group_list, alpm_list_count(group_list), group_cmp);
		for (; group_list; group_list = group_list->next) {
			alpm_group_t *group = group_list->data;
			gtk_tree_store_append(main_window_gui.repo_tree_store, &child, &toplevel);
			gtk_tree_store_set(main_window_gui.repo_tree_store, &child,
				FILTERS_COL_ICON, "gtk-file",
				FILTERS_COL_TITLE, group->name,
				FILTERS_COL_MASK, HIDE_NONE,
				FILTERS_COL_DB, db,
				FILTERS_COL_GROUP, group,
				-1
			);
		}
	}

	gtk_tree_store_append(main_window_gui.repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		main_window_gui.repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, "gtk-harddisk",
		FILTERS_COL_TITLE, _("Foreign"),
		FILTERS_COL_MASK, HIDE_NATIVE,
		-1
	);
}

static void block_signal_package_treeview_selection(gboolean block)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window_gui.package_treeview));

	if (block) {
		g_signal_handler_block(selection, pkg_selchange_handler_id);
	} else {
		g_signal_handler_unblock(selection, pkg_selchange_handler_id);
	}
}

static void repo_row_selected(GtkTreeSelection *selection, gpointer user_data)
{
	GtkTreeModel *repo_model, *package_model;
	GtkTreeIter repo_iter, package_iter;
	GtkTreeSelection *pkg_selection;
	guint filters;
	alpm_db_t *db = NULL;
	alpm_group_t *group = NULL;

	if (gtk_tree_selection_get_selected(selection, &repo_model, &repo_iter)) {
		/* if any package list row is selected then unselect it */
		pkg_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window_gui.package_treeview));
		if (gtk_tree_selection_get_selected(pkg_selection, &package_model, &package_iter)) {
			gtk_tree_selection_unselect_iter(pkg_selection, &package_iter);
		}

		/* clear any open package details */
		show_package(NULL);

		/* prevent selecting a different repo row while we're filtering */
		block_signal_package_treeview_selection(TRUE);

		/* get row data from model */
		gtk_tree_model_get(repo_model, &repo_iter, FILTERS_COL_MASK, &filters, -1);
		gtk_tree_model_get(repo_model, &repo_iter, FILTERS_COL_DB, &db, -1);
		gtk_tree_model_get(repo_model, &repo_iter, FILTERS_COL_GROUP, &group, -1);

		/* set filters */
		package_filters.status_filter = filters;
		package_filters.group = group;
		package_filters.db = db;

		/* trigger refilter of package list */
		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(package_list_model));

		/* release selection blocking */
		block_signal_package_treeview_selection(FALSE);
	}
}

static gboolean row_visible(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	install_reason_t reason;
	gchar *db_name;
	alpm_pkg_t *pkg;

	/* get row data from model */
	gtk_tree_model_get(model, iter, PACKAGES_COL_STATUS, &reason, -1);
	gtk_tree_model_get(model, iter, PACAKGES_COL_REPO, &db_name, -1);
	gtk_tree_model_get(model, iter, PACKAGES_COL_PKG, &pkg, -1);

	/* find any filters that would exclude this row */
	if (package_filters.status_filter & HIDE_INSTALLED) {
		if (reason != PKG_REASON_NOT_INSTALLED) return FALSE;
	}
	if (package_filters.status_filter & HIDE_UNINSTALLED) {
		if (reason == PKG_REASON_NOT_INSTALLED) return FALSE;
	}
	if (package_filters.status_filter & HIDE_EXPLICIT) {
		if (reason == PKG_REASON_EXPLICIT) return FALSE;
	}
	if (package_filters.status_filter & HIDE_DEPEND) {
		if (reason == PKG_REASON_DEPEND) return FALSE;
	}
	if (package_filters.status_filter & HIDE_OPTION) {
		if (reason == PKG_REASON_OPTIONAL) return FALSE;
	}
	if (package_filters.status_filter & HIDE_NATIVE) {
		if (g_strcmp0(db_name, "local") != 0) return FALSE;
	}
	if (package_filters.status_filter & HIDE_FOREIGN) {
		if (g_strcmp0(db_name, "local") == 0) return FALSE;
	}
	if (package_filters.db != NULL) {
		if (g_strcmp0(db_name, alpm_db_get_name(package_filters.db)) != 0) return FALSE;
	}
	if (package_filters.group != NULL) {
		alpm_list_t *pkg_groups = alpm_pkg_get_groups(pkg);
		if (alpm_list_count(pkg_groups) == 0) {
			return FALSE;
		} else {
			if (!alpm_list_find(pkg_groups, package_filters.group, group_cmp_find)) return FALSE;
		}
	}

	g_free(db_name);

	return TRUE;
}

static void activate_about(GSimpleAction *simple, GVariant *parameter, gpointer user_data)
{
	show_about_dialog(main_window_gui.window);
}

static void activate_quit(GSimpleAction *simple, GVariant *parameter, gpointer user_data)
{
	exit(0);
}

static GActionGroup *create_action_group(void)
{
	const GActionEntry entries[] = {
		{ "about", activate_about, NULL, NULL, NULL, { 0, 0, 0 } },
		{ "quit", activate_quit, NULL, NULL, NULL, { 0, 0, 0 } }
	};

	GSimpleActionGroup *group;

	group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(group), entries, G_N_ELEMENTS(entries), NULL);

	return G_ACTION_GROUP(group);
}

static GMenuModel *create_app_menu(void)
{
	GMenu *section, *menu;

	menu = g_menu_new();

	section = g_menu_new();
	/* l10n: header menu items */
	g_menu_insert(section, 0, _("About PacFinder"), "app.about");
	g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
	g_object_unref(section);

	section = g_menu_new();
	g_menu_insert(section, 0, _("Quit"), "app.quit");
	g_menu_append_section(menu, NULL, G_MENU_MODEL(section));
	g_object_unref(section);

	return G_MENU_MODEL(menu);
}

static void create_main_menu(void)
{
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(main_window_gui.menu_button), create_app_menu());
	gtk_widget_insert_action_group(main_window_gui.menu_button, "app", create_action_group());
}

static void bind_events_to_widgets(void)
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window_gui.repo_treeview));
	g_signal_connect(
		G_OBJECT(selection),
		"changed",
		G_CALLBACK(repo_row_selected),
		NULL
	);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window_gui.package_treeview));
	pkg_selchange_handler_id = g_signal_connect(
		G_OBJECT(selection),
		"changed",
		G_CALLBACK(package_row_selected),
		NULL
	);
}

void initialize_main_window(void)
{
	create_main_menu();
	populate_db_tree_view();
	show_package_list();
	show_package(NULL);
	gtk_tree_model_filter_set_visible_func(
		GTK_TREE_MODEL_FILTER(package_list_model),
		(GtkTreeModelFilterVisibleFunc)row_visible,
		NULL,
		NULL
	);
	bind_events_to_widgets();
	gtk_window_set_icon_name(GTK_WINDOW(main_window_gui.window), APPLICATION_ID);
}
