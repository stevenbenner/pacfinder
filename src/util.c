/* util.c - PacFinder general utility functions
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

#include "util.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <sys/types.h>

gchar *human_readable_size(off_t size)
{
	/* l10n: file size units */
	const gchar *sizes[] = { N_("B"), N_("KiB"), N_("MiB"), N_("GiB"), N_("TiB") };
	guint size_index = 0;
	gfloat file_size = size;

	while (file_size >= 1024 && size_index < 4) {
		size_index++;
		file_size = file_size / 1024;
	}

	/* l10n: file size pattern - %.2f is the numeric value (e.g. 8.88) and %s is
	 * the localized unit (e.g. "MiB"), the order must remain the same */
	return g_strdup_printf(_("%.2f %s"), file_size, _(sizes[size_index]));
}
