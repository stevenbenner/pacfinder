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

#ifndef PF_INTERFACE_H
#define PF_INTERFACE_H

#include <gtk/gtk.h>

enum {
	FILTERS_COL_ICON = 0,
	FILTERS_COL_TITLE,
	FILTERS_COL_DB,
	FILTERS_COL_GROUP,
	FILTERS_NUM_COLS
};

struct details_overview_t {
	GtkWidget *status_image;
	GtkWidget *heading_label;
	GtkWidget *desc_label;
	GtkWidget *left_label;
	GtkWidget *middle_label;
	GtkWidget *right_label;
	GtkWidget *required_by_label;
	GtkWidget *optional_for_label;
	GtkWidget *dependencies_label;
};

struct main_window_gui_t {
	GtkWidget *window;
	GtkWidget *menu_button;
	GtkWidget *repo_treeview;
	GtkWidget *package_treeview;
	GtkTreeStore *repo_tree_store;
	GtkListStore *package_list_store;
	GtkNotebook *details_notebook;
	struct details_overview_t details_overview;
	GtkListStore *package_details_list_store;
};
extern struct main_window_gui_t main_window_gui;

void create_app_window(GtkApplication *app);

#endif /* PF_INTERFACE_H */
