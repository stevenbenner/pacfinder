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

#include "main.h"

#include <gtk/gtk.h>
#include <unistd.h>

#include "interface.h"
#include "window.h"

static void on_activate_app(GtkApplication *app, gpointer user_data)
{
	create_app_window(app);
	initialize_main_window();
}

int main(int argc, char **argv)
{
	GtkApplication *app;
	int status;

	/* prevent users from running as root */
	if (geteuid() == 0) {
		g_error("Do not run this program as root.");
	}

	/* launch gtk application */
	app = gtk_application_new(APPLICATION_ID, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate_app), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
