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

#ifndef PF_DATABASE_H
#define PF_DATABASE_H

#include <alpm.h>
#include <glib.h>

typedef enum {
	PKG_REASON_NOT_INSTALLED = 0,
	PKG_REASON_EXPLICIT,
	PKG_REASON_DEPEND,
	PKG_REASON_OPTIONAL
} install_reason_t;

extern alpm_list_t *foreign_pkg_list;

int package_cmp(const void *p1, const void *p2);
int group_cmp(const void *p1, const void *p2);
int group_cmp_find(const void *p1, const void *p2);

alpm_handle_t *get_alpm_handle(void);
alpm_list_t *get_all_packages(void);
install_reason_t get_pkg_status(alpm_pkg_t *pkg);
void database_free(void);

#endif /* PF_DATABASE_H */
