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

#include "settings.h"

#include <gdk/gdk.h>
#include <gio/gio.h>

#include "main.h"

#define WINDOW_MIN_WIDTH 600
#define WINDOW_MIN_HEIGHT 450

static GSettings *settings = NULL;

void initialize_settings(void)
{
	if (settings == NULL) {
		settings = g_settings_new(APPLICATION_ID);
	}
}

void free_settings(void)
{
	if (settings != NULL) {
		g_object_unref(settings);
	}
}

GdkRectangle get_saved_window_geometry(void)
{
	GdkRectangle geometry = { 0 };

	g_settings_get(settings, "window-position", "(ii)", &geometry.x, &geometry.y);
	g_settings_get(settings, "window-size", "(ii)", &geometry.width, &geometry.height);

	geometry.width = MAX(geometry.width, WINDOW_MIN_WIDTH);
	geometry.height = MAX(geometry.height, WINDOW_MIN_HEIGHT);

	return geometry;
}

void set_saved_window_geometry(GdkRectangle geometry)
{
	geometry.width = MAX(geometry.width, WINDOW_MIN_WIDTH);
	geometry.height = MAX(geometry.height, WINDOW_MIN_HEIGHT);

    g_settings_set(settings, "window-size", "(ii)", geometry.width, geometry.height);
    g_settings_set(settings, "window-position", "(ii)", geometry.x, geometry.y);
}

gboolean get_saved_window_state(void)
{
	gboolean maximized = FALSE;

	g_settings_get(settings, "window-maximized", "b", &maximized);

	return maximized;
}

void set_saved_window_state(const gboolean maximized)
{
	g_settings_set_boolean(settings, "window-maximized", maximized);
}
