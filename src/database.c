/* database.c - PacFinder alpm data access layer state and utility functions
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
#include "database.h"

/* system libraries */
#include <alpm.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glob.h>
#include <sys/types.h>

#define FS_ROOT_PATH "/"
#define PACMAN_CONFIG_PATH "/etc/pacman.conf"
#define PACMAN_DB_PATH "/var/lib/pacman/"
#define MAX_CONFIG_DEPTH 5

alpm_list_t *foreign_pkg_list = NULL;

static alpm_handle_t *handle = NULL;
static alpm_db_t *db_local = NULL;
static alpm_list_t *all_packages_list = NULL;

static gint register_syncs(const gchar *file_path, const gint depth)
{
	gboolean ret;
	gchar *contents = NULL;
	gsize length;
	gchar **lines = NULL;
	guint i;

	/* prevent recursion loops by limiting max depth */
	if (depth >= MAX_CONFIG_DEPTH) {
		return 1;
	}

	ret = g_file_get_contents(file_path, &contents, &length, NULL);

	if (ret) {
		lines = g_strsplit(contents, "\n", -1);

		for (i = 0; lines[i] != NULL; i++) {
			gchar *section = NULL;
			alpm_db_t *db;
			gchar **pair = NULL;
			glob_t globstruct;
			size_t x;

			g_strstrip(lines[i]);

			if (g_str_has_prefix(lines[i], "[") && g_str_has_suffix(lines[i], "]")) {
				/* handle sections: sections other than "options" are dbs */
				section = g_strndup(&lines[i][1], strlen(lines[i]) - 2);
				if (g_strcmp0(section, "options") != 0) {
					db = alpm_register_syncdb(handle, section, ALPM_SIG_USE_DEFAULT);
					alpm_db_set_usage(db, ALPM_DB_USAGE_ALL);
				}
				g_free(section);
			} else {
				/* handle key=val pairs: find "Include" directives and recurse conf files */
				pair = g_strsplit(lines[i], "=", 2);
				if (pair && g_strv_length(pair) == 2) {
					g_strstrip(pair[0]);
					if (g_strcmp0(pair[0], "Include") == 0) {
						g_strstrip(pair[1]);
						if (glob(pair[1], GLOB_ERR, NULL, &globstruct) == 0) {
							for (x = 0; x < globstruct.gl_pathc; x++) {
								register_syncs(globstruct.gl_pathv[x], depth + 1);
							}
						}
						globfree(&globstruct);
					}
				}
				g_strfreev(pair);
			}
		}
		g_strfreev(lines);
	} else {
		/* l10n: error message shown in cli or log */
		g_error(_("Failed to read pacman config file: %s"), file_path);
	}

	g_free(contents);

	return ret;
}

static void initialize_alpm(void)
{
	alpm_errno_t err;

	handle = alpm_initialize(FS_ROOT_PATH, PACMAN_DB_PATH, &err);
	if (!handle) {
		/* l10n: error message shown in cli or log */
		g_error(_("Failed to initialize libalpm: %s"), alpm_strerror(err));
	}
	register_syncs(PACMAN_CONFIG_PATH, 0);
}

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

alpm_handle_t *get_alpm_handle(void)
{
	if (handle == NULL) {
		initialize_alpm();
	}
	return handle;
}

alpm_db_t *get_local_db(void)
{
	if (db_local == NULL) {
		db_local = alpm_get_localdb(get_alpm_handle());
	}
	return db_local;
}

alpm_list_t *get_all_packages(void)
{
	alpm_list_t *i;
	alpm_db_t *db_local;

	/* collect all packages from the syncdbs */
	if (all_packages_list == NULL) {
		for(i = alpm_get_syncdbs(get_alpm_handle()); i; i = i->next) {
			alpm_db_t *db = i->data;
			alpm_list_t *db_package_list = alpm_db_get_pkgcache(db);

			if (db_package_list != NULL) {
				all_packages_list = alpm_list_join(
					all_packages_list,
					alpm_list_copy(db_package_list)
				);
			}
		}
	}

	/* iterate the localdb packages and find any that are not listed in the
	 * syncdbs - when found, add it to the "all packages" list as well as keep
	 * track of them in the "foreign" packages list */
	db_local = get_local_db();
	for (i = alpm_db_get_pkgcache(db_local); i; i = i->next) {
		alpm_pkg_t *pkg = i->data;

		if (!alpm_list_find(all_packages_list, pkg, package_cmp)) {
			foreign_pkg_list = alpm_list_add(foreign_pkg_list, pkg);
			all_packages_list = alpm_list_add(all_packages_list, pkg);
		}
	}

	/* sort the final list */
	all_packages_list = alpm_list_msort(
		all_packages_list,
		alpm_list_count(all_packages_list),
		package_cmp
	);

	return all_packages_list;
}

alpm_pkg_t *find_package(const gchar *pkg_name)
{
	return alpm_pkg_find(all_packages_list, pkg_name);
}

alpm_pkg_t *find_satisfier(const gchar *dep_str)
{
	return alpm_find_satisfier(all_packages_list, dep_str);
}

install_reason_t get_pkg_status(alpm_pkg_t *pkg)
{
	static const install_reason_t reason_map[] = {
		[ALPM_PKG_REASON_EXPLICIT] = PKG_REASON_EXPLICIT,
		[ALPM_PKG_REASON_DEPEND] = PKG_REASON_DEPEND
	};

	install_reason_t ret;
	alpm_db_t *db_local;
	alpm_pkg_t *local_pkg;
	alpm_pkgreason_t install_reason;
	alpm_list_t *required_by, *optional_for;

	ret = PKG_REASON_NOT_INSTALLED;

	db_local = get_local_db();
	local_pkg = alpm_db_get_pkg(db_local, alpm_pkg_get_name(pkg));

	if (local_pkg != NULL) {
		install_reason = alpm_pkg_get_reason(local_pkg);
		required_by = alpm_pkg_compute_requiredby(local_pkg);

		if (install_reason == ALPM_PKG_REASON_DEPEND && alpm_list_count(required_by) == 0) {
			optional_for = alpm_pkg_compute_optionalfor(local_pkg);

			if (alpm_list_count(optional_for) == 0) {
				ret = PKG_REASON_ORPHAN;
			} else {
				ret = PKG_REASON_OPTIONAL;
			}

			alpm_list_free_inner(optional_for, g_free);
			alpm_list_free(optional_for);
		} else {
			ret = reason_map[install_reason];
		}

		alpm_list_free_inner(required_by, g_free);
		alpm_list_free(required_by);
	}

	return ret;
}

void database_free(void)
{
	if (handle) {
		alpm_list_free(all_packages_list);
		alpm_list_free(foreign_pkg_list);
		all_packages_list = NULL;
		foreign_pkg_list = NULL;
		if (alpm_release(handle) == -1) {
			/* l10n: error message shown in cli or log */
			g_error(_("Failed to release libalpm."));
		}
		db_local = NULL;
		handle = NULL;
	}
}
