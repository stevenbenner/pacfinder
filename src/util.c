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

/* build config */
#include "config.h"

/* file header */
#include "util.h"

/* system libraries */
#include <alpm.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <sys/types.h>

static GPtrArray *list_to_ptrarray(alpm_list_t *list)
{
	alpm_list_t *i;
	GPtrArray *arr;

	arr = g_ptr_array_new_with_free_func(g_free);

	for (i = list; i; i = alpm_list_next(i)) {
		g_ptr_array_add(arr, g_strdup(i->data));
	}
	g_ptr_array_add(arr, NULL);

	return arr;
}

gchar *list_to_string(alpm_list_t *list)
{
	GPtrArray *arr;
	gchar *str;

	arr = list_to_ptrarray(list);
	str = g_strjoinv(", ", (gchar **)(arr->pdata));

	g_ptr_array_free(arr, TRUE);

	return str;
}

gchar *deplist_to_string(alpm_list_t *list)
{
	alpm_list_t *string_list, *i;
	gchar *str;

	string_list = NULL;

	for (i = list; i; i = alpm_list_next(i)) {
		gchar *dep_str = alpm_dep_compute_string(i->data);
		string_list = alpm_list_add(string_list, dep_str);
	}

	str = list_to_string(string_list);

	alpm_list_free_inner(string_list, g_free);
	alpm_list_free(string_list);

	return str;
}

gchar *human_readable_size(const off_t size)
{
	/* l10n: file size units */
	static const gchar *sizes[] = { N_("B"), N_("KiB"), N_("MiB"), N_("GiB"), N_("TiB") };
	guint size_index = 0;
	gfloat file_size = size;

	while (file_size >= 1024 && size_index < 4) {
		size_index++;
		file_size = file_size / 1024;
	}

	if (size >= 1024) {
		/* l10n: file size patterns - %.2f or %ld is the numeric value (e.g. 8.88 or 8)
		 * and %s is the localized unit (e.g. "MiB"), the order must remain the same */
		return g_strdup_printf(_("%.2f %s"), file_size, _(sizes[size_index]));
	} else {
		return g_strdup_printf(_("%ld %s"), size, _(sizes[size_index]));
	}
}

gchar *strtrunc_dep_desc(const gchar *str)
{
	gchar *name, *desc;

	g_return_val_if_fail(str != NULL, NULL);

	name = g_strdup(str);
	desc = strstr(name, ": ");

	if (desc != NULL) {
		name[desc - name] = '\0';
	}

	return name;
}

int package_cmp(const void *p1, const void *p2) {
	alpm_pkg_t *pkg1 = (alpm_pkg_t *)p1;
	alpm_pkg_t *pkg2 = (alpm_pkg_t *)p2;

	return g_strcmp0(alpm_pkg_get_name(pkg1), alpm_pkg_get_name(pkg2));
}

int group_cmp(const void *p1, const void *p2) {
	const alpm_group_t *grp1 = p1;
	const alpm_group_t *grp2 = p2;

	return g_strcmp0(grp1->name, grp2->name);
}

int group_cmp_find(const void *p1, const void *p2) {
	const alpm_group_t *grp1 = (alpm_group_t *)&p1;
	const alpm_group_t *grp2 = p2;

	return g_strcmp0(grp1->name, grp2->name);
}
