/* aboutdialog.c - PacFinder about dialog
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

#include "config.h"

#include "aboutdialog.h"

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "main.h"

static const gchar *authors[] = {
	"Steven Benner",
	NULL
};

void show_about_dialog(GtkWindow *parent)
{
	/* l10n: program version shown in about dialog - %s is version number (e.g. 1.2) */
	gchar *version_str = g_strdup_printf(_("Version %s"), VERSION);
	/* l10n: program copyright shown in about dialog - %s is year (e.g. 2022, or 2022-2023) */
	gchar *copyright_str = g_strdup_printf(_("Copyright %s Steven Benner"), "2022");

	gtk_show_about_dialog(
		parent,
		/* l10n: program name shown in about dialog */
		"program-name", _("PacFinder"),
		"icon-name", APPLICATION_ID,
		"logo-icon-name", APPLICATION_ID,
		"version", version_str,
		/* l10n: program description shown in about dialog */
		"comments", _("Repository & package explorer for Arch Linux."),
		/* l10n: program web site link label shown in about dialog */
		"website-label", _("PacFinder on GitHub"),
		"website", PACKAGE_URL,
		"copyright", copyright_str,
		"authors", authors,
		"license-type", GTK_LICENSE_APACHE_2_0,
		NULL
	);

	g_free(copyright_str);
	g_free(version_str);
}
