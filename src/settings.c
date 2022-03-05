/* settings.c - PacFinder global settings state and interaction
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

/* file header */
#include "settings.h"

/* system libraries */
#include <gdk/gdk.h>
#include <gio/gio.h>

/* pacfinder */
#include "main.h"

#define LEFT_PANED_MIN 50
#define LEFT_PANED_MAX 1000
#define RIGHT_PANED_MIN 50
#define RIGHT_PANED_MAX 1000
#define WINDOW_MIN_WIDTH 600
#define WINDOW_MIN_HEIGHT 450

static GSettings *settings = NULL;

void initialize_settings(void)
{
	if (settings == NULL) {
		settings = g_settings_new(APPLICATION_ID);
	}
}

void settings_free(void)
{
	if (settings != NULL) {
		g_settings_sync();
		g_clear_object(&settings);
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

gint get_saved_left_width(void)
{
	return g_settings_get_int(settings, "left-width");
}

void set_saved_left_width(gint width)
{
	width = MIN(width, LEFT_PANED_MAX);
	width = MAX(width, LEFT_PANED_MIN);

	g_settings_set_int(settings, "left-width", width);
}

gint get_saved_right_height(void)
{
	return g_settings_get_int(settings, "right-height");
}

void set_saved_right_height(gint height)
{
	height = MIN(height, RIGHT_PANED_MAX);
	height = MAX(height, RIGHT_PANED_MIN);

	g_settings_set_int(settings, "right-height", height);
}

void get_saved_package_column_widths(gint *w1, gint *w2, gint *w3, gint *w4)
{
	g_settings_get(settings, "package-list-column-widths", "(iiii)", w1, w2, w3, w4);
}

void set_saved_package_column_widths(const gint w1, const gint w2, const gint w3, const gint w4)
{
	g_settings_set(settings, "package-list-column-widths", "(iiii)", w1, w2, w3, w4);
}
