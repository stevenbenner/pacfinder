/* main.c - PacFinder main application entry point and supporting code
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
#include "main.h"

/* system libraries */
#include <alpm.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <locale.h>
#include <unistd.h>

/* pacfinder */
#include "interface.h"
#include "settings.h"
#include "window.h"

static struct app_options_t {
	gboolean show_version;
} app_options;

static void init_i18n(void)
{
#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif
}

static void init_cli_options(GApplication *app, struct app_options_t *options)
{
	const GOptionEntry cli_options[] = {
		{
			.long_name = "version",
			.short_name = 'v',
			.flags = G_OPTION_FLAG_NONE,
			.arg = G_OPTION_ARG_NONE,
			.arg_data = &(options->show_version),
			/* l10n: cli option descriptions */
			.description = N_("Show version information"),
			.arg_description = NULL
		},
		{ NULL }
	};

	g_application_add_main_option_entries(app, cli_options);
}

static void on_activate_app(GtkApplication *app, struct app_options_t *options)
{
	if (options->show_version == TRUE) {
		/* l10n: cli --version output */
		g_print(_("PacFinder - version %s"), VERSION);
		g_print("\n\n");
		g_print(_("Copyright %s Steven Benner"), COPYRIGHT_YEAR);
		g_print("\n\n");
		g_print(_("Report bugs to <%s>."), PACKAGE_BUGREPORT);
		g_print("\n");
		return;
	}

	/* add icons from gresource */
	gtk_icon_theme_add_resource_path(
		gtk_icon_theme_get_default(),
		"/com/stevenbenner/pacfinder/icons"
	);

	create_app_window(app);
	initialize_main_window();
}

int main(int argc, char **argv)
{
	const gchar *current_desktop = g_getenv("XDG_CURRENT_DESKTOP");
	GtkApplication *app;
	int status;

	/* set up internationalization */
	init_i18n();

	/* log informational message to help with diagnostics */
	g_message(
		"PacFinder %s (libalpm %s, GTK %d.%d.%d, %s)",
		PACKAGE_VERSION,
		alpm_version(),
		gtk_get_major_version(),
		gtk_get_minor_version(),
		gtk_get_micro_version(),
		current_desktop ? current_desktop : "n/a"
	);

	/* prevent users from running as root */
	if (geteuid() == 0) {
		/* l10n: error message shown in cli or log */
		g_error(_("Do not run this program as root."));
	}

	/* initialize settings system */
	initialize_settings();

	/* launch gtk application */
	app = gtk_application_new(APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate_app), &app_options);
	init_cli_options(G_APPLICATION(app), &app_options);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
