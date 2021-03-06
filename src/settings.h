/* settings.h
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

#ifndef PF_SETTINGS_H
#define PF_SETTINGS_H

#include <gdk/gdk.h>

void initialize_settings(void);
void settings_free(void);
GdkRectangle get_saved_window_geometry(void);
void set_saved_window_geometry(GdkRectangle geometry);
gboolean get_saved_window_state(void);
void set_saved_window_state(const gboolean maximized);
gint get_saved_left_width(void);
void set_saved_left_width(gint width);
gint get_saved_right_height(void);
void set_saved_right_height(gint height);
void get_saved_package_column_widths(gint *w1, gint *w2, gint *w3, gint *w4);
void set_saved_package_column_widths(const gint w1, const gint w2, const gint w3, const gint w4);

#endif /* PF_SETTINGS_H */
