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

#include "database.h"

#include <alpm.h>
#include <glib.h>

static alpm_handle_t *handle = NULL;
static alpm_list_t *all_packages_list = NULL;

static gint register_syncs(void)
{
	const gchar *file_name = "/etc/pacman.conf";
	const alpm_siglevel_t level = ALPM_SIG_USE_DEFAULT;

	gboolean ret;
	gchar *contents = NULL;
	gsize length;
	gchar **lines = NULL;
	guint i;

	ret = g_file_get_contents(file_name, &contents, &length, NULL);

	if (ret) {
		lines = g_strsplit(contents, "\n", -1);

		for (i = 0; lines[i] != NULL; i++) {
			gchar *section = NULL;
			alpm_db_t *db;

			g_strstrip(lines[i]);

			if (g_str_has_prefix(lines[i], "[") && g_str_has_suffix(lines[i], "]")) {
				section = g_strndup(&lines[i][1], strlen(lines[i]) - 2);
				if (g_strcmp0(section, "options") != 0) {
					db = alpm_register_syncdb(handle, section, level);
					alpm_db_set_usage(db, ALPM_DB_USAGE_ALL);
				}
				g_free(section);
			}
		}
	} else {
		g_error("Failed to read pacman config file: %s", file_name);
	}

	g_strfreev(lines);
	g_free(contents);

	return ret;
}

static void initialize_alpm(void)
{
	alpm_errno_t err;

	handle = alpm_initialize("/", "/var/lib/pacman/", &err);
	if (!handle) {
		g_error("Failed to initalize libalpm: %s", alpm_strerror(err));
	}
	register_syncs();
}

alpm_handle_t *get_alpm_handle(void)
{
	if (handle == NULL) {
		initialize_alpm();
	}
	return handle;
}

alpm_list_t *get_all_packages(void)
{
	alpm_list_t *i;

	if (all_packages_list == NULL) {
		for(i = alpm_get_syncdbs(get_alpm_handle()); i; i = i->next) {
			alpm_db_t *db = i->data;
			alpm_list_t *db_package_list = alpm_db_get_pkgcache(db);

			if (db_package_list != NULL) {
				all_packages_list = alpm_list_join(all_packages_list, db_package_list);
			}
		}
	}

	return all_packages_list;
}

install_reason_t get_pkg_status(alpm_pkg_t *pkg)
{
	static install_reason_t const reason_map[] = {
		[ALPM_PKG_REASON_EXPLICIT] = PKG_REASON_EXPLICIT,
		[ALPM_PKG_REASON_DEPEND] = PKG_REASON_DEPEND
	};

	install_reason_t ret;
	alpm_db_t *db_local;
	alpm_pkg_t *local_pkg;
	alpm_pkgreason_t install_reason;
	alpm_list_t *required_by;

	ret = PKG_REASON_NOT_INSTALLED;

	db_local = alpm_get_localdb(get_alpm_handle());
	local_pkg = alpm_db_get_pkg(db_local, alpm_pkg_get_name(pkg));

	if (local_pkg != NULL) {
		install_reason = alpm_pkg_get_reason(local_pkg);
		required_by = alpm_pkg_compute_requiredby(local_pkg);

		if (install_reason == ALPM_PKG_REASON_DEPEND && alpm_list_count(required_by) == 0) {
			ret =  PKG_REASON_OPTIONAL;
		} else {
			ret =  reason_map[install_reason];
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
		if (alpm_release(handle) == -1) {
			g_error("Failed to release libalpm!");
		}
	}
}
