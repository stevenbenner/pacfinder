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

#ifndef PF_SETTINGS_H
#define PF_SETTINGS_H

#include <gdk/gdk.h>

void initialize_settings(void);
void free_settings(void);
GdkRectangle get_saved_window_geometry(void);
void set_saved_window_geometry(GdkRectangle geometry);
gboolean get_saved_window_state(void);
void set_saved_window_state(const gboolean maximized);

#endif /* PF_SETTINGS_H */
