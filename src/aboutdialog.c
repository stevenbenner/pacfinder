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

#include "aboutdialog.h"

#include <gtk/gtk.h>

#include "main.h"

static const gchar *authors[] = {
	"Steven Benner",
	NULL
};

void show_about_dialog(GtkWidget *parent)
{
	gchar *version_str = g_strdup_printf("Version %s", VERSION);

	gtk_show_about_dialog(
		GTK_WINDOW(parent),
		"program-name", "PacFinder",
		"version", version_str,
		"comments", "Repository & package explorer for Arch Linux.",
		"website-label", "PacFinder on GitHub",
		"website", PACKAGE_URL,
		"copyright", "Copyright 2022 Steven Benner",
		"authors", authors,
		"license-type", GTK_LICENSE_APACHE_2_0,
		NULL
	);

	g_free(version_str);
}
