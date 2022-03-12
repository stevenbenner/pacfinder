/* window.c - PacFinder main window business logic
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
#include "window.h"

/* system libraries */
#include <alpm.h>
#include <gdk/gdk.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <sys/types.h>

/* pacfinder */
#include "aboutdialog.h"
#include "database.h"
#include "interface.h"
#include "main.h"
#include "settings.h"
#include "util.h"

/* package list filtering */
enum {
	HIDE_NONE = 0,
	HIDE_INSTALLED = (1 << 0),
	HIDE_UNINSTALLED = (1 << 1),
	HIDE_EXPLICIT = (1 << 2),
	HIDE_DEPEND = (1 << 3),
	HIDE_OPTION = (1 << 4),
	HIDE_ORPHAN = (1 << 5),
	HIDE_NATIVE = (1 << 6),
	HIDE_FOREIGN = (1 << 7)
};

/* package list filters */
static struct {
	guint status_filter;
	alpm_db_t *db;
	alpm_group_t *group;
	gchar *search_string;
} package_filters;

/* local variables */
static gulong pkg_selchange_handler_id;
static gulong search_changed_handler_id;

static void show_package(alpm_pkg_t *pkg);

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
				"icon-explicit",
				GTK_ICON_SIZE_DIALOG
			);
			break;
		case PKG_REASON_DEPEND:
			gtk_image_set_from_icon_name(
				GTK_IMAGE(main_window_gui.details_overview.status_image),
				"icon-depend",
				GTK_ICON_SIZE_DIALOG
			);
			break;
		case PKG_REASON_OPTIONAL:
			gtk_image_set_from_icon_name(
				GTK_IMAGE(main_window_gui.details_overview.status_image),
				"icon-optional",
				GTK_ICON_SIZE_DIALOG
			);
			break;
		case PKG_REASON_ORPHAN:
			gtk_image_set_from_icon_name(
				GTK_IMAGE(main_window_gui.details_overview.status_image),
				"icon-orphan",
				GTK_ICON_SIZE_DIALOG
			);
			break;
		case PKG_REASON_NOT_INSTALLED:
			gtk_image_set_from_icon_name(
				GTK_IMAGE(main_window_gui.details_overview.status_image),
				"icon-uninstalled",
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
	gtk_label_set_markup(main_window_gui.details_overview.heading_label, str);
	g_free(str);

	gtk_label_set_label(main_window_gui.details_overview.desc_label, alpm_pkg_get_desc(pkg));

	str = g_strdup_printf("<a href=\"%s\">%s</a>", alpm_pkg_get_url(pkg), alpm_pkg_get_url(pkg));
	gtk_label_set_markup(main_window_gui.details_overview.link_label, str);
	g_free(str);

	str = human_readable_size(alpm_pkg_get_isize(pkg));
	gtk_label_set_label(main_window_gui.details_overview.left_label, str);
	g_free(str);
	gtk_label_set_label(main_window_gui.details_overview.middle_label, alpm_pkg_get_arch(pkg));
	gtk_label_set_label(main_window_gui.details_overview.right_label, alpm_db_get_name(alpm_pkg_get_db(pkg)));

	dep_list = alpm_pkg_compute_requiredby(pkg);
	count = alpm_list_count(dep_list);
	/* l10n: package dependency counts - %ld will be a number (zero or more) */
	str = g_strdup_printf(ngettext("%ld package", "%ld packages", count), count);
	gtk_label_set_markup(main_window_gui.details_overview.required_by_label, str);
	g_free(str);
	alpm_list_free_inner(dep_list, g_free);
	alpm_list_free(dep_list);

	dep_list = alpm_pkg_compute_optionalfor(pkg);
	count = alpm_list_count(dep_list);
	str = g_strdup_printf(ngettext("%ld package", "%ld packages", count), count);
	gtk_label_set_markup(main_window_gui.details_overview.optional_for_label, str);
	g_free(str);
	alpm_list_free_inner(dep_list, g_free);
	alpm_list_free(dep_list);

	dep_list = alpm_pkg_get_depends(pkg);
	count = alpm_list_count(dep_list);
	str = g_strdup_printf(ngettext("%ld package", "%ld packages", count), count);
	gtk_label_set_markup(main_window_gui.details_overview.dependencies_label, str);
	g_free(str);
}

static void on_dep_clicked(GtkButton* self, alpm_depend_t *user_data)
{
	alpm_pkg_t *pkg = find_satisfier(user_data->name);
	if (pkg != NULL) {
		show_package(pkg);
		gtk_notebook_set_current_page(main_window_gui.details_notebook, 0);
	}
}

static void on_deppkg_clicked(GtkButton* self, alpm_depend_t *user_data)
{
	alpm_pkg_t *pkg = (alpm_pkg_t *)user_data;
	if (pkg != NULL) {
		show_package(pkg);
		gtk_notebook_set_current_page(main_window_gui.details_notebook, 0);
	}
}

static GtkWidget *create_dep_button(alpm_pkg_t *pkg, const gchar *label)
{
	gchar *name, *desc;
	GtkWidget *button, *image;

	/* handle labels created with alpm_dep_compute_string() by
	 * removing the description from the string */
	name = g_strdup(label);
	desc = g_strrstr(name, ": ");
	if (desc != NULL) {
		name[desc - name] = '\0';
	}

	button = gtk_button_new_with_label(name);

	switch (get_pkg_status(pkg)) {
		case PKG_REASON_EXPLICIT:
			image = gtk_image_new_from_icon_name("icon-explicit", GTK_ICON_SIZE_BUTTON);
			break;
		case PKG_REASON_DEPEND:
			image = gtk_image_new_from_icon_name("icon-depend", GTK_ICON_SIZE_BUTTON);
			break;
		case PKG_REASON_OPTIONAL:
			image = gtk_image_new_from_icon_name("icon-optional", GTK_ICON_SIZE_BUTTON);
			break;
		case PKG_REASON_ORPHAN:
			image = gtk_image_new_from_icon_name("icon-orphan", GTK_ICON_SIZE_BUTTON);
			break;
		case PKG_REASON_NOT_INSTALLED:
		default:
			image = gtk_image_new_from_icon_name("icon-uninstalled", GTK_ICON_SIZE_BUTTON);
	}

	gtk_button_set_image(GTK_BUTTON(button), image);
	gtk_button_set_always_show_image(GTK_BUTTON(button), TRUE);

	g_free(name);

	return button;
}

static void show_package_deps(alpm_pkg_t *pkg)
{
	alpm_list_t *i;
	gint row;

	/* empty the dependencies boxes of any previous children */
	gtk_container_foreach(
		GTK_CONTAINER(main_window_gui.package_details_deps_box),
		(void *)gtk_widget_destroy,
		NULL
	);
	gtk_container_foreach(
		GTK_CONTAINER(main_window_gui.package_details_opts_grid),
		(void *)gtk_widget_destroy,
		NULL
	);

	/* append required dependencies */
	for (i = alpm_pkg_get_depends(pkg); i; i = alpm_list_next(i)) {
		alpm_depend_t *dep;
		gchar *dep_str;
		GtkWidget *button;

		dep = i->data;
		dep_str = alpm_dep_compute_string(dep);

		button = create_dep_button(find_satisfier(dep_str), dep_str);
		g_signal_connect(button, "clicked", G_CALLBACK(on_dep_clicked), dep);

		gtk_flow_box_insert(main_window_gui.package_details_deps_box, button, -1);

		g_free(dep_str);
	}

	/* append optional dependencies */
	row = 0;
	for (i = alpm_pkg_get_optdepends(pkg); i; i = alpm_list_next(i)) {
		alpm_depend_t *dep;
		gchar *dep_str;
		GtkWidget *button, *label;

		dep = i->data;
		dep_str = alpm_dep_compute_string(dep);

		button = create_dep_button(find_satisfier(dep_str), dep_str);
		gtk_widget_set_margin_start(button, 5);
		gtk_widget_set_margin_top(button, 5);
		gtk_widget_set_margin_bottom(button, 5);
		g_signal_connect(button, "clicked", G_CALLBACK(on_dep_clicked), dep);

		label = gtk_label_new(dep->desc);
		gtk_widget_set_halign(label, GTK_ALIGN_START);
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_label_set_xalign(GTK_LABEL(label), 0);

		gtk_grid_attach(main_window_gui.package_details_opts_grid, button, 0, row, 1, 1);
		gtk_grid_attach(main_window_gui.package_details_opts_grid, label, 1, row, 1, 1);

		row++;

		g_free(dep_str);
	}

	gtk_widget_show_all(GTK_WIDGET(main_window_gui.package_details_deps_box));
	gtk_widget_show_all(GTK_WIDGET(main_window_gui.package_details_opts_grid));
}

static void show_package_depsfor(alpm_pkg_t *pkg)
{
	alpm_list_t *required_by, *optional_for, *i;
	gint row;

	/* empty the dependents boxes of any previous children */
	gtk_container_foreach(
		GTK_CONTAINER(main_window_gui.package_details_depsfor_box),
		(void *)gtk_widget_destroy,
		NULL
	);
	gtk_container_foreach(
		GTK_CONTAINER(main_window_gui.package_details_optsfor_grid),
		(void *)gtk_widget_destroy,
		NULL
	);

	/* append required by dependents */
	required_by = alpm_pkg_compute_requiredby(pkg);
	for (i = required_by; i; i = alpm_list_next(i)) {
		alpm_pkg_t *dep;
		GtkWidget *button;

		dep = find_package(i->data);
		button = create_dep_button(dep, alpm_pkg_get_name(dep));
		g_signal_connect(button, "clicked", G_CALLBACK(on_deppkg_clicked), dep);

		gtk_flow_box_insert(main_window_gui.package_details_depsfor_box, button, -1);
	}
	alpm_list_free_inner(required_by, g_free);
	alpm_list_free(required_by);

	/* append optional for dependents */
	optional_for = alpm_pkg_compute_optionalfor(pkg);
	row = 0;
	for (i = optional_for; i; i = alpm_list_next(i)) {
		alpm_pkg_t *dep;
		alpm_list_t *dep_opts;
		gchar *dep_desc;
		GtkWidget *button, *label;

		dep = find_satisfier(i->data);
		dep_opts = alpm_pkg_get_optdepends(dep);
		dep_desc = NULL;

		/* find the matching optional dependency in the dependent to
		 * get the description */
		for (; dep_opts; dep_opts = alpm_list_next(dep_opts)) {
			alpm_depend_t *opt;
			gchar *opt_name;
			alpm_list_t *prov_list;

			opt = dep_opts->data;
			opt_name = opt->name;

			/* simple name comparison */
			if (g_strcmp0(opt_name, alpm_pkg_get_name(pkg)) == 0) {
				dep_desc = opt->desc;
				break;
			}

			/* check all provides for a match */
			prov_list = alpm_pkg_get_provides(pkg);
			for (; prov_list; prov_list = alpm_list_next(prov_list)) {
				alpm_depend_t *prov;

				prov = prov_list->data;

				if (g_strcmp0(opt_name, prov->name) == 0) {
					dep_desc = opt->desc;
					break;
				}
			}

			/* if we found a match in the provides then we're done */
			if (dep_desc != NULL) {
				break;
			}
		}

		button = create_dep_button(dep, alpm_pkg_get_name(dep));
		gtk_widget_set_margin_start(button, 5);
		gtk_widget_set_margin_top(button, 5);
		gtk_widget_set_margin_bottom(button, 5);
		g_signal_connect(button, "clicked", G_CALLBACK(on_deppkg_clicked), dep);

		label = gtk_label_new(dep_desc);
		gtk_widget_set_halign(label, GTK_ALIGN_START);
		gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
		gtk_label_set_xalign(GTK_LABEL(label), 0);

		gtk_grid_attach(main_window_gui.package_details_optsfor_grid, button, 0, row, 1, 1);
		gtk_grid_attach(main_window_gui.package_details_optsfor_grid, label, 1, row, 1, 1);

		row++;
	}
	alpm_list_free_inner(optional_for, g_free);
	alpm_list_free(optional_for);

	gtk_widget_show_all(GTK_WIDGET(main_window_gui.package_details_depsfor_box));
	gtk_widget_show_all(GTK_WIDGET(main_window_gui.package_details_optsfor_grid));
}

static void append_details_row(GtkTreeIter *iter, const gchar *name, const gchar *value)
{
	gtk_list_store_append(main_window_gui.package_details_list_store, iter);
	gtk_list_store_set(
		main_window_gui.package_details_list_store,
		iter,
		DETAILS_COL_NAME, name,
		DETAILS_COL_VALUE, value,
		-1
	);
}

static void show_package_details(alpm_pkg_t *pkg)
{
	alpm_db_t *db_local;
	alpm_pkg_t *local_pkg;
	gchar *licenses_str, *groups_str, *provides_str, *dependson_str, *optionals_str,
	      *requiredby_str, *optionalfor_str, *conflicts_str, *replaces_str, *fsize_str, *isize_str,
	      *bdate_str, *idate_str;
	alpm_list_t *requiredby, *optionalfor;
	GDateTime *bdate, *idate;
	GtkTreeIter iter;

	/* grab local package, if it exists */
	db_local = get_local_db();
	local_pkg = alpm_db_get_pkg(db_local, alpm_pkg_get_name(pkg));

	/* compute dependency lists */
	requiredby = alpm_pkg_compute_requiredby(pkg);
	optionalfor = alpm_pkg_compute_optionalfor(pkg);

	/* gather dates */
	bdate = g_date_time_new_from_unix_local(alpm_pkg_get_builddate(pkg));
	if (local_pkg != NULL) {
		idate = g_date_time_new_from_unix_local(alpm_pkg_get_installdate(local_pkg));
	} else {
		idate = NULL;
	}

	/* build display strings */
	licenses_str = list_to_string(alpm_pkg_get_licenses(pkg));
	groups_str = list_to_string(alpm_pkg_get_groups(pkg));
	provides_str = deplist_to_string(alpm_pkg_get_provides(pkg));
	dependson_str = deplist_to_string(alpm_pkg_get_depends(pkg));
	optionals_str = deplist_to_string(alpm_pkg_get_optdepends(pkg));
	requiredby_str = list_to_string(requiredby);
	optionalfor_str = list_to_string(optionalfor);
	conflicts_str = deplist_to_string(alpm_pkg_get_conflicts(pkg));
	replaces_str = deplist_to_string(alpm_pkg_get_replaces(pkg));
	fsize_str = human_readable_size(alpm_pkg_get_size(pkg));
	isize_str = human_readable_size(alpm_pkg_get_isize(pkg));
	bdate_str = g_date_time_format_iso8601(bdate);
	if (idate != NULL) {
		idate_str = g_date_time_format_iso8601(idate);
	} else {
		idate_str = "";
	}

	/* clean up dates */
	g_date_time_unref(bdate);
	if (idate != NULL) g_date_time_unref(idate);

	/* clean up dependency lists */
	alpm_list_free_inner(requiredby, g_free);
	alpm_list_free_inner(optionalfor, g_free);
	alpm_list_free(requiredby);
	alpm_list_free(optionalfor);

	/* empty list from any previously selected package */
	gtk_list_store_clear(main_window_gui.package_details_list_store);

	/* add detail rows */
	/* l10n: package details tab row labels */
	append_details_row(&iter, _("Name:"), alpm_pkg_get_name(pkg));
	append_details_row(&iter, _("Version:"), alpm_pkg_get_version(pkg));
	append_details_row(&iter, _("Description:"), alpm_pkg_get_desc(pkg));
	append_details_row(&iter, _("Architecture:"), alpm_pkg_get_arch(pkg));
	append_details_row(&iter, _("URL:"), alpm_pkg_get_url(pkg));
	append_details_row(&iter, _("Licenses:"), licenses_str);
	append_details_row(&iter, _("Groups:"), groups_str);
	append_details_row(&iter, _("Provides:"), provides_str);
	append_details_row(&iter, _("Depends On:"), dependson_str);
	append_details_row(&iter, _("Optional:"), optionals_str);
	append_details_row(&iter, _("Required By:"), requiredby_str);
	append_details_row(&iter, _("Optional For:"), optionalfor_str);
	append_details_row(&iter, _("Conflicts:"), conflicts_str);
	append_details_row(&iter, _("Replaces:"), replaces_str);
	append_details_row(&iter, _("File Size:"), fsize_str);
	append_details_row(&iter, _("Install Size:"), isize_str);
	append_details_row(&iter, _("Packager:"), alpm_pkg_get_packager(pkg));
	append_details_row(&iter, _("Build Date:"), bdate_str);
	append_details_row(&iter, _("Install Date:"), idate_str);

	/* clean up display strings */
	g_free(licenses_str);
	g_free(groups_str);
	g_free(provides_str);
	g_free(dependson_str);
	g_free(optionals_str);
	g_free(requiredby_str);
	g_free(optionalfor_str);
	g_free(conflicts_str);
	g_free(replaces_str);
	g_free(fsize_str);
	g_free(isize_str);
	g_free(bdate_str);
	if (idate != NULL) g_free(idate_str);
}

static void show_package(alpm_pkg_t *pkg)
{
	if (pkg == NULL) {
		gtk_widget_hide(GTK_WIDGET(main_window_gui.details_notebook));
		return;
	}

	show_package_overview(pkg);
	show_package_deps(pkg);
	show_package_depsfor(pkg);
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

static void populate_db_tree_view(GtkTreeStore *repo_tree_store)
{
	GtkIconTheme *icon_theme;
	GdkPixbuf *icon;
	GtkTreeIter toplevel, child;
	alpm_list_t *i;

	icon_theme = gtk_icon_theme_get_default();

	/* add standard filter lists */
	icon = gtk_icon_theme_load_icon(icon_theme, "go-home", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, icon,
		/* l10n: filter names shown in main filter list */
		FILTERS_COL_TITLE, _("All Packages"),
		FILTERS_COL_MASK, HIDE_NONE,
		-1
	);
	g_object_unref(icon);

	icon = gtk_icon_theme_load_icon(icon_theme, "icon-installed", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, icon,
		FILTERS_COL_TITLE, _("Installed"),
		FILTERS_COL_MASK, HIDE_UNINSTALLED,
		-1
	);
	g_object_unref(icon);

	icon = gtk_icon_theme_load_icon(icon_theme, "icon-explicit", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, icon,
		FILTERS_COL_TITLE, _("Explicit"),
		FILTERS_COL_MASK, HIDE_UNINSTALLED | HIDE_DEPEND | HIDE_OPTION | HIDE_ORPHAN,
		-1
	);
	g_object_unref(icon);

	icon = gtk_icon_theme_load_icon(icon_theme, "icon-depend", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, icon,
		FILTERS_COL_TITLE, _("Dependency"),
		FILTERS_COL_MASK, HIDE_UNINSTALLED | HIDE_EXPLICIT | HIDE_OPTION | HIDE_ORPHAN,
		-1
	);
	g_object_unref(icon);

	icon = gtk_icon_theme_load_icon(icon_theme, "icon-optional", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, icon,
		FILTERS_COL_TITLE, _("Optional"),
		FILTERS_COL_MASK, HIDE_UNINSTALLED | HIDE_EXPLICIT | HIDE_DEPEND | HIDE_ORPHAN,
		-1
	);
	g_object_unref(icon);

	icon = gtk_icon_theme_load_icon(icon_theme, "icon-orphan", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, icon,
		FILTERS_COL_TITLE, _("Orphan"),
		FILTERS_COL_MASK, HIDE_UNINSTALLED | HIDE_EXPLICIT | HIDE_DEPEND | HIDE_OPTION,
		-1
	);
	g_object_unref(icon);

	/* add known databases */
	for (i = alpm_get_syncdbs(get_alpm_handle()); i; i = i->next) {
		alpm_db_t *db;
		alpm_list_t *group_list;

		db = i->data;

		icon = gtk_icon_theme_load_icon(icon_theme, "folder", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
		gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
		gtk_tree_store_set(
			repo_tree_store,
			&toplevel,
			FILTERS_COL_ICON, icon,
			FILTERS_COL_TITLE, alpm_db_get_name(db),
			FILTERS_COL_MASK, HIDE_NONE,
			FILTERS_COL_DB, db,
			-1
		);
		g_object_unref(icon);

		/* add any groups found for this database */
		group_list = alpm_list_copy(alpm_db_get_groupcache(db));
		group_list = alpm_list_msort(group_list, alpm_list_count(group_list), group_cmp);
		for (; group_list; group_list = group_list->next) {
			alpm_group_t *group = group_list->data;
			icon = gtk_icon_theme_load_icon(icon_theme, "text-x-generic", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
			gtk_tree_store_append(repo_tree_store, &child, &toplevel);
			gtk_tree_store_set(
				repo_tree_store,
				&child,
				FILTERS_COL_ICON, icon,
				FILTERS_COL_TITLE, group->name,
				FILTERS_COL_MASK, HIDE_NONE,
				FILTERS_COL_DB, db,
				FILTERS_COL_GROUP, group,
				-1
			);
			g_object_unref(icon);
		}
		alpm_list_free(group_list);
	}

	icon = gtk_icon_theme_load_icon(icon_theme, "drive-harddisk", 16, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
	gtk_tree_store_append(repo_tree_store, &toplevel, NULL);
	gtk_tree_store_set(
		repo_tree_store,
		&toplevel,
		FILTERS_COL_ICON, icon,
		FILTERS_COL_TITLE, _("Foreign"),
		FILTERS_COL_MASK, HIDE_NATIVE,
		-1
	);
	g_object_unref(icon);
}

static void unselect_package(void)
{
	GtkTreePath *path;
	GtkTreeSelection *selection;

	/* reset tree view cursor to top */
	path = gtk_tree_path_new_from_indices(0, -1);
	gtk_tree_view_set_cursor(
		main_window_gui.package_treeview,
		path,
		NULL,
		FALSE
	);
	gtk_tree_path_free(path);

	/* if any package list row is selected then unselect it */
	selection = gtk_tree_view_get_selection(main_window_gui.package_treeview);
	gtk_tree_selection_unselect_all(selection);

	/* clear any open package details */
	show_package(NULL);
}

static void block_signal_package_treeview_selection(gboolean block)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection(main_window_gui.package_treeview);

	if (block) {
		g_signal_handler_block(selection, pkg_selchange_handler_id);
	} else {
		g_signal_handler_unblock(selection, pkg_selchange_handler_id);
	}
}

static void block_signal_search_changed(gboolean block)
{
	if (block) {
		g_signal_handler_block(main_window_gui.search_entry, search_changed_handler_id);
	} else {
		g_signal_handler_unblock(main_window_gui.search_entry, search_changed_handler_id);
	}
}

static void load_data(void)
{
	/* close package view */
	show_package(NULL);

	/* load repo tree */
	populate_db_tree_view(main_window_gui.repo_tree_store);

	/* load package list */
	show_package_list();
}

static void repo_row_selected(GtkTreeSelection *selection, gpointer user_data)
{
	GtkTreeModel *repo_model;
	GtkTreeIter repo_iter;
	guint filters;
	alpm_db_t *db = NULL;
	alpm_group_t *group = NULL;

	if (gtk_tree_selection_get_selected(selection, &repo_model, &repo_iter)) {
		/* prevent selecting a different repo row while we're filtering */
		block_signal_package_treeview_selection(TRUE);

		/* get row data from model */
		gtk_tree_model_get(
			repo_model,
			&repo_iter,
			FILTERS_COL_MASK, &filters,
			FILTERS_COL_DB, &db,
			FILTERS_COL_GROUP, &group,
			-1
		);

		/* set filters */
		package_filters.status_filter = filters;
		package_filters.group = group;
		package_filters.db = db;

		/* trigger refilter of package list */
		gtk_tree_model_filter_refilter(main_window_gui.package_list_model);

		/* release selection blocking */
		block_signal_package_treeview_selection(FALSE);

		/* if any package list row is selected then unselect it */
		unselect_package();
	}
}

static gboolean is_package_filtered(const install_reason_t reason, const gchar *db_name, alpm_pkg_t *pkg)
{
	if (package_filters.status_filter & HIDE_INSTALLED) {
		if (reason != PKG_REASON_NOT_INSTALLED) {
			return TRUE;
		}
	}

	if (package_filters.status_filter & HIDE_UNINSTALLED) {
		if (reason == PKG_REASON_NOT_INSTALLED) {
			return TRUE;
		}
	}

	if (package_filters.status_filter & HIDE_EXPLICIT) {
		if (reason == PKG_REASON_EXPLICIT) {
			return TRUE;
		}
	}

	if (package_filters.status_filter & HIDE_DEPEND) {
		if (reason == PKG_REASON_DEPEND) {
			return TRUE;
		}
	}

	if (package_filters.status_filter & HIDE_OPTION) {
		if (reason == PKG_REASON_OPTIONAL) {
			return TRUE;
		}
	}

	if (package_filters.status_filter & HIDE_ORPHAN) {
		if (reason == PKG_REASON_ORPHAN) {
			return TRUE;
		}
	}

	if (package_filters.status_filter & HIDE_NATIVE) {
		if (g_strcmp0(db_name, "local") != 0) {
			return TRUE;
		}
	}

	if (package_filters.status_filter & HIDE_FOREIGN) {
		if (g_strcmp0(db_name, "local") == 0) {
			return TRUE;
		}
	}

	if (package_filters.db != NULL) {
		if (g_strcmp0(db_name, alpm_db_get_name(package_filters.db)) != 0) {
			return TRUE;
		}
	}

	if (package_filters.group != NULL) {
		alpm_list_t *pkg_groups = alpm_pkg_get_groups(pkg);
		if (alpm_list_count(pkg_groups) == 0) {
			return TRUE;
		} else if (!alpm_list_find(pkg_groups, package_filters.group, group_cmp_find)) {
			return TRUE;
		}
	}

	if (package_filters.search_string != NULL) {
		if (g_strrstr(alpm_pkg_get_name(pkg), package_filters.search_string) == NULL) {
			return TRUE;
		}
	}

	/* no matches, row not filtered */
	return FALSE;
}

static gboolean row_visible(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	install_reason_t reason;
	gchar *db_name;
	alpm_pkg_t *pkg;
	gboolean ret;

	/* get row data from model */
	gtk_tree_model_get(
		model,
		iter,
		PACKAGES_COL_STATUS, &reason,
		PACAKGES_COL_REPO, &db_name,
		PACKAGES_COL_PKG, &pkg,
		-1
	);

	/* check if the filters exclude this row */
	ret = !is_package_filtered(reason, db_name, pkg);

	g_free(db_name);

	return ret;
}

static void on_search_changed(GtkSearchEntry *entry, gpointer user_data)
{
	/* block search events to prevent repeated search invocations */
	block_signal_search_changed(TRUE);

	/* prevent selecting a different repo row while we're filtering */
	block_signal_package_treeview_selection(TRUE);

	/* set filters */
	package_filters.status_filter = HIDE_NONE;
	package_filters.group = NULL;
	package_filters.db = NULL;
	package_filters.search_string = g_ascii_strdown(gtk_entry_get_text(GTK_ENTRY(entry)), -1);

	/* trigger refilter of package list */
	gtk_tree_model_filter_refilter(main_window_gui.package_list_model);

	/* clean up */
	g_free(package_filters.search_string);
	package_filters.search_string = NULL;

	/* if any package list row is selected then deselect it */
	unselect_package();

	/* release selection blocking */
	block_signal_package_treeview_selection(FALSE);

	/* release search block */
	block_signal_search_changed(FALSE);
}

static void activate_about(GSimpleAction *simple, GVariant *parameter, gpointer user_data)
{
	show_about_dialog(main_window_gui.window);
}

static void activate_quit(GSimpleAction *simple, GVariant *parameter, gpointer user_data)
{
	exit(EXIT_SUCCESS);
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

static void create_main_menu(GtkWidget *menu_button)
{
	gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menu_button), create_app_menu());
	gtk_widget_insert_action_group(menu_button, "app", create_action_group());
}

static void on_window_realize(GtkWindow *window)
{
	GdkRectangle geometry;
	gint width1, width2, width3, width4;
	GtkTreeViewColumn *column;

	/* set window geometry */
	geometry = get_saved_window_geometry();
	gtk_window_set_default_size(window, geometry.width, geometry.height);
	if (geometry.x > -1 && geometry.y > -1) {
		gtk_window_move(window, geometry.x, geometry.y);
	}
	if (get_saved_window_state()) {
		gtk_window_maximize(window);
	}

	/* set paned positions */
	gtk_paned_set_position(main_window_gui.hpaned, get_saved_left_width());
	gtk_paned_set_position(main_window_gui.vpaned, get_saved_right_height());

	/* set column widths */
	get_saved_package_column_widths(&width1, &width2, &width3, &width4);
	column = gtk_tree_view_get_column(main_window_gui.package_treeview, PACKAGES_COL_NAME);
	gtk_tree_view_column_set_fixed_width(column, width1);
	column = gtk_tree_view_get_column(main_window_gui.package_treeview, PACKAGES_COL_VERSION);
	gtk_tree_view_column_set_fixed_width(column, width2);
	column = gtk_tree_view_get_column(main_window_gui.package_treeview, PACKAGES_COL_STATUS);
	gtk_tree_view_column_set_fixed_width(column, width3);
	column = gtk_tree_view_get_column(main_window_gui.package_treeview, PACAKGES_COL_REPO);
	gtk_tree_view_column_set_fixed_width(column, width4);
}

static gboolean on_window_configure(GtkWindow *window, GdkEventConfigure *event)
{
	GdkRectangle geometry = { 0 };
	gboolean maximized;

	gtk_window_get_size(window, &geometry.width, &geometry.height);
	gtk_window_get_position(window, &geometry.x, &geometry.y);
	maximized = gtk_window_is_maximized(window);

	if (!maximized) {
		set_saved_window_geometry(geometry);
	}
	set_saved_window_state(maximized);

	return GDK_EVENT_PROPAGATE;
}

static void on_window_destroy(GtkWindow *window)
{
	settings_free();
	database_free();
}

static void bind_events_to_window(GtkWindow *window)
{
	g_signal_connect(window, "realize", G_CALLBACK(on_window_realize), NULL);
	g_signal_connect(window, "configure-event", G_CALLBACK(on_window_configure), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
}

static void save_package_column_widths(GtkTreeView *package_treeview)
{
	GtkTreeViewColumn *column;
	gint width1, width2, width3, width4;

	column = gtk_tree_view_get_column(package_treeview, PACKAGES_COL_NAME);
	width1 = gtk_tree_view_column_get_width(column);

	column = gtk_tree_view_get_column(package_treeview, PACKAGES_COL_VERSION);
	width2 = gtk_tree_view_column_get_width(column);

	column = gtk_tree_view_get_column(package_treeview, PACKAGES_COL_STATUS);
	width3 = gtk_tree_view_column_get_width(column);

	column = gtk_tree_view_get_column(package_treeview, PACAKGES_COL_REPO);
	width4 = gtk_tree_view_column_get_width(column);

	set_saved_package_column_widths(width1, width2, width3, width4);
}

static gboolean on_paned_reposition(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	set_saved_left_width(gtk_paned_get_position(main_window_gui.hpaned));
	set_saved_right_height(gtk_paned_get_position(main_window_gui.vpaned));
	save_package_column_widths(main_window_gui.package_treeview);

	return FALSE;
}

static void bind_events_to_widgets(void)
{
	GtkTreeSelection *selection;

	/* filter list item selected */
	selection = gtk_tree_view_get_selection(main_window_gui.repo_treeview);
	g_signal_connect(
		G_OBJECT(selection),
		"changed",
		G_CALLBACK(repo_row_selected),
		NULL
	);

	/* package list item selected */
	selection = gtk_tree_view_get_selection(main_window_gui.package_treeview);
	pkg_selchange_handler_id = g_signal_connect(
		G_OBJECT(selection),
		"changed",
		G_CALLBACK(package_row_selected),
		NULL
	);

	/* search entry changed */
	search_changed_handler_id = g_signal_connect(
		main_window_gui.search_entry,
		"search-changed",
		G_CALLBACK(on_search_changed),
		NULL
	);

	/* paned position change */
	g_signal_connect(
		main_window_gui.hpaned,
		"accept-position",
		G_CALLBACK(on_paned_reposition),
		NULL
	);
	g_signal_connect(
		main_window_gui.hpaned,
		"button-release-event",
		G_CALLBACK(on_paned_reposition),
		NULL
	);

}

void initialize_main_window(void)
{
	create_main_menu(main_window_gui.menu_button);

	bind_events_to_window(main_window_gui.window);
	bind_events_to_widgets();
	gtk_tree_model_filter_set_visible_func(
		main_window_gui.package_list_model,
		(GtkTreeModelFilterVisibleFunc)row_visible,
		NULL,
		NULL
	);

	load_data();

	gtk_window_set_icon_name(main_window_gui.window, APPLICATION_ID);
	gtk_widget_show_all(GTK_WIDGET(main_window_gui.window));
}
