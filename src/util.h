/* util.h
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

#ifndef PF_UTIL_H
#define PF_UTIL_H

#include <alpm.h>
#include <glib.h>
#include <sys/types.h>

gchar *list_to_string(const alpm_list_t *list);
gchar *deplist_to_string(alpm_list_t *list);
gchar *human_readable_size(const off_t size);
gchar *strtrunc_dep_desc(const gchar *str);
int package_cmp(const void *p1, const void *p2);
int group_cmp(const void *p1, const void *p2);
int group_cmp_find(const void *p1, const void *p2);

#endif /* PF_UTIL_H */
