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

#include "window.h"

#include <alpm.h>
#include <gtk/gtk.h>

#include "database.h"
#include "interface.h"

/* package list filtering */
enum {
	HIDE_NONE = 0,
	HIDE_INSTALLED = (1 << 0),
	HIDE_UNINSTALLED = (1 << 1),
	HIDE_EXPLICIT = (1 << 2),
	HIDE_DEPEND = (1 << 3),
	HIDE_OPTION = (1 << 4)
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

		gtk_list_store_append(package_list_store, &iter);
		gtk_list_store_set(
			package_list_store,
			&iter,
			0, alpm_pkg_get_name(pkg),
			1, alpm_pkg_get_version(pkg),
			2, get_pkg_status(pkg),
			3, alpm_db_get_name(alpm_pkg_get_db(pkg)),
			4, pkg,
			-1
		);
	}

	package_list_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(package_list_store), NULL);
	gtk_tree_view_set_model(GTK_TREE_VIEW(package_treeview), GTK_TREE_MODEL(package_list_model));
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(package_list_store), 0, GTK_SORT_ASCENDING);
}

static void show_package_details(alpm_pkg_t *pkg)
{
	GtkTreeIter iter;

	/* empty list from any previously selected package */
	gtk_list_store_clear(package_details_list_store);

	if (pkg == NULL) {
		return;
	}

	/* add detail rows */
	gtk_list_store_append(package_details_list_store, &iter);
	gtk_list_store_set(package_details_list_store, &iter, 0, "Name:", 1, alpm_pkg_get_name(pkg), -1);
	gtk_list_store_append(package_details_list_store, &iter);
	gtk_list_store_set(package_details_list_store, &iter, 0, "Version:", 1, alpm_pkg_get_version(pkg), -1);
	gtk_list_store_append(package_details_list_store, &iter);
	gtk_list_store_set(package_details_list_store, &iter, 0, "Description:", 1, alpm_pkg_get_desc(pkg), -1);
	gtk_list_store_append(package_details_list_store, &iter);
	gtk_list_store_set(package_details_list_store, &iter, 0, "Architecture:", 1, alpm_pkg_get_arch(pkg), -1);
	gtk_list_store_append(package_details_list_store, &iter);
	gtk_list_store_set(package_details_list_store, &iter, 0, "URL:", 1, alpm_pkg_get_url(pkg), -1);
	gtk_list_store_append(package_details_list_store, &iter);
	gtk_list_store_set(package_details_list_store, &iter, 0, "Packager:", 1, alpm_pkg_get_packager(pkg), -1);
}

static void package_row_selected(GtkTreeSelection *selection, gpointer user_data)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	alpm_pkg_t *pkg;

	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
		gtk_tree_model_get(model, &iter, 4, &pkg, -1);
		show_package_details(pkg);
	}
}

static void populate_db_tree_view(void)
{
	GtkTreeIter toplevel, child;
	alpm_list_t *i;

	/* add standard filter lists */
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(repo_tree_store, &toplevel, 0, "gtk-home", 1, "All Packages", -1);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(repo_tree_store, &toplevel, 0, "gtk-media-play", 1, "Installed", -1);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(repo_tree_store, &toplevel, 0, "gtk-yes", 1, "Explicit", -1);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(repo_tree_store, &toplevel, 0, "gtk-leave-fullscreen", 1, "Dependancy", -1);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(repo_tree_store, &toplevel, 0, "gtk-connect", 1, "Optional", -1);

	/* add known databases */
	for (i = alpm_get_syncdbs(get_alpm_handle()); i; i = i->next) {
		alpm_db_t *db;
		alpm_list_t *j;

		db = i->data;

		gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
		gtk_tree_store_set(repo_tree_store, &toplevel,
			0, "gtk-directory",
			1, alpm_db_get_name(db),
			2, db,
			-1
		);

		/* add any groups found for this database */
		for (j = alpm_db_get_groupcache(db); j; j = j->next) {
			alpm_group_t *group = j->data;
			gtk_tree_store_append(repo_tree_store, &child, &toplevel);
			gtk_tree_store_set(repo_tree_store, &child,
				0, "gtk-file",
				1, group->name,
				2, db,
				3, group,
				-1
			);
		}
	}
}

static void block_signal_package_treeview_selection(gboolean block)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(package_treeview));

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
	gchar *repo_name;
	alpm_db_t *db;
	alpm_group_t *group = NULL;

	if (gtk_tree_selection_get_selected(selection, &repo_model, &repo_iter)) {
		/* if any package list row is selected then unselect it */
		pkg_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(package_treeview));
		if (gtk_tree_selection_get_selected(pkg_selection, &package_model, &package_iter)) {
			gtk_tree_selection_unselect_iter(pkg_selection, &package_iter);
		}

		/* clear any open package details */
		show_package_details(NULL);

		/* prevent selecting a different repo row while we're filtering */
		block_signal_package_treeview_selection(TRUE);

		/* get row data from model */
		gtk_tree_model_get(repo_model, &repo_iter, 1, &repo_name, -1);
		gtk_tree_model_get(repo_model, &repo_iter, 2, &db, -1);
		gtk_tree_model_get(repo_model, &repo_iter, 3, &group, -1);

		/* reset filters */
		package_filters.status_filter = HIDE_NONE;
		package_filters.group = NULL;
		package_filters.db = NULL;

		/* set filters based on selection */
		if (g_strcmp0(repo_name, "Installed") == 0) {
			package_filters.status_filter = HIDE_UNINSTALLED;
		} else if (g_strcmp0(repo_name, "Explicit") == 0) {
			package_filters.status_filter = HIDE_UNINSTALLED | HIDE_DEPEND | HIDE_OPTION;
		} else if (g_strcmp0(repo_name, "Dependancy") == 0) {
			package_filters.status_filter = HIDE_UNINSTALLED | HIDE_EXPLICIT | HIDE_OPTION;
		} else if (g_strcmp0(repo_name, "Optional") == 0) {
			package_filters.status_filter = HIDE_UNINSTALLED | HIDE_EXPLICIT | HIDE_DEPEND;
		} else if (group != NULL) {
			package_filters.group = group;
			package_filters.db = db;
		} else if (db != NULL) {
			package_filters.db = db;
		}

		g_free(repo_name);

		gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(package_list_model));

		block_signal_package_treeview_selection(FALSE);
	}
}

static int group_cmp(const void *p1, const void *p2) {
	const alpm_group_t *grp1 = (alpm_group_t *)&p1;
	const alpm_group_t *grp2 = p2;

	return g_strcmp0(grp1->name, grp2->name);
}

static gboolean row_visible(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	install_reason_t reason;
	gchar *db_name;
	alpm_pkg_t *pkg;

	/* get row data from model */
	gtk_tree_model_get(model, iter, 2, &reason, -1);
	gtk_tree_model_get(model, iter, 3, &db_name, -1);
	gtk_tree_model_get(model, iter, 4, &pkg, -1);

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
	if (package_filters.db != NULL) {
		if (g_strcmp0(db_name, alpm_db_get_name(package_filters.db)) != 0) return FALSE;
	}
	if (package_filters.group != NULL) {
		alpm_list_t *pkg_groups = alpm_pkg_get_groups(pkg);
		if (alpm_list_count(pkg_groups) == 0) {
			return FALSE;
		} else {
			if (!alpm_list_find(pkg_groups, package_filters.group, group_cmp)) return FALSE;
		}
	}

	g_free(db_name);

	return TRUE;
}

static void bind_events_to_widgets(void)
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(repo_treeview));
	g_signal_connect(
		G_OBJECT(selection),
		"changed",
		G_CALLBACK(repo_row_selected),
		NULL
	);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(package_treeview));
	pkg_selchange_handler_id = g_signal_connect(
		G_OBJECT(selection),
		"changed",
		G_CALLBACK(package_row_selected),
		NULL
	);
}

void initialize_main_window(void)
{
	populate_db_tree_view();
	show_package_list();
	gtk_tree_model_filter_set_visible_func(
		GTK_TREE_MODEL_FILTER(package_list_model),
		(GtkTreeModelFilterVisibleFunc)row_visible,
		NULL,
		NULL
	);
	bind_events_to_widgets();
}
