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

extern GtkWidget *repo_treeview;
extern GtkWidget *package_treeview;

extern GtkTreeStore *repo_tree_store;
extern GtkListStore *package_list_store;
extern GtkListStore *package_details_list_store;

void create_app_window(GtkApplication *);

#endif /* PF_INTERFACE_H */
