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

#include "util.h"

#include <glib.h>
#include <sys/types.h>

gchar *human_readable_size(off_t size)
{
	const gchar *sizes[] = { "B", "KiB", "MiB", "GiB", "TiB" };
	guint size_index = 0;
	gfloat file_size = size;

	while (file_size >= 1024 && size_index < 4) {
		size_index++;
		file_size = file_size / 1024;
	}

	return g_strdup_printf("%.2f %s", file_size, sizes[size_index]);
}
