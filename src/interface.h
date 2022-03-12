/* interface.h
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

#ifndef PF_INTERFACE_H
#define PF_INTERFACE_H

#include <gtk/gtk.h>

enum {
	FILTERS_COL_ICON = 0,
	FILTERS_COL_TITLE,
	FILTERS_COL_MASK,
	FILTERS_COL_DB,
	FILTERS_COL_GROUP,
	FILTERS_NUM_COLS
};

enum {
	PACKAGES_COL_NAME = 0,
	PACKAGES_COL_VERSION,
	PACKAGES_COL_STATUS,
	PACAKGES_COL_REPO,
	PACKAGES_COL_PKG,
	PACKAGES_NUM_COLS
};

enum {
	DETAILS_COL_NAME = 0,
	DETAILS_COL_VALUE,
	DETAILS_NUM_COLS
};

struct details_overview_t {
	GtkWidget *status_image;
	GtkLabel *heading_label;
	GtkLabel *desc_label;
	GtkLabel *link_label;
	GtkLabel *left_label;
	GtkLabel *middle_label;
	GtkLabel *right_label;
	GtkLabel *required_by_label;
	GtkLabel *optional_for_label;
	GtkLabel *dependencies_label;
};

struct main_window_gui_t {
	GtkWindow *window;
	GtkWidget *search_entry;
	GtkWidget *menu_button;
	GtkPaned *hpaned;
	GtkPaned *vpaned;
	GtkTreeView *repo_treeview;
	GtkTreeView *package_treeview;
	GtkTreeStore *repo_tree_store;
	GtkListStore *package_list_store;
	GtkTreeModelFilter *package_list_model;
	GtkNotebook *details_notebook;
	struct details_overview_t details_overview;
	GtkFlowBox *package_details_deps_box;
	GtkGrid *package_details_opts_grid;
	GtkFlowBox *package_details_depsfor_box;
	GtkGrid *package_details_optsfor_grid;
	GtkListStore *package_details_list_store;
};

extern struct main_window_gui_t main_window_gui;

void create_app_window(GtkApplication *app);

#endif /* PF_INTERFACE_H */
