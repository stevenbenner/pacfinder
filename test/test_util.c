/* test_util.c
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

#include "test_util.h"

#include <alpm.h>
#include <glib.h>
#include <math.h>

#include "util.h"

static void test_list_to_string(void)
{
	alpm_list_t *empty_list = NULL;
	alpm_list_t *list_of_one = NULL;
	alpm_list_t *list_of_three = NULL;

	list_of_one = alpm_list_add(list_of_one, "item 1");

	list_of_three = alpm_list_add(list_of_three, "item 1");
	list_of_three = alpm_list_add(list_of_three, "item 2");
	list_of_three = alpm_list_add(list_of_three, "item 3");

	gchar *null_items = list_to_string(empty_list);
	gchar *one_item = list_to_string(list_of_one);
	gchar *three_items = list_to_string(list_of_three);

	g_assert_cmpstr(null_items, ==, "");
	g_assert_cmpstr(one_item, ==, "item 1");
	g_assert_cmpstr(three_items, ==, "item 1, item 2, item 3");

	g_free(null_items);
	g_free(one_item);
	g_free(three_items);
	alpm_list_free(empty_list);
	alpm_list_free(list_of_one);
	alpm_list_free(list_of_three);
}

static void test_human_readable_size(void)
{
	gchar *zero = human_readable_size(0);
	gchar *ten_b = human_readable_size(10);
	gchar *max_b = human_readable_size(1023);
	gchar *one_k = human_readable_size(1024);
	gchar *one_point_five_k = human_readable_size(1024 * 1.5);
	gchar *one_m = human_readable_size(pow(1024, 2));
	gchar *one_point_nine_nine_m = human_readable_size((pow(1024, 2) * 2) - (1024 * 10));
	gchar *one_g = human_readable_size(pow(1024, 3));
	gchar *one_t = human_readable_size(pow(1024, 4));
	gchar *one_p = human_readable_size(pow(1024, 5));

	g_assert_cmpstr(zero, ==, "0 B");
	g_assert_cmpstr(ten_b, ==, "10 B");
	g_assert_cmpstr(max_b, ==, "1023 B");
	g_assert_cmpstr(one_k, ==, "1.00 KiB");
	g_assert_cmpstr(one_point_five_k, ==, "1.50 KiB");
	g_assert_cmpstr(one_m, ==, "1.00 MiB");
	g_assert_cmpstr(one_point_nine_nine_m, ==, "1.99 MiB");
	g_assert_cmpstr(one_g, ==, "1.00 GiB");
	g_assert_cmpstr(one_t, ==, "1.00 TiB");
	g_assert_cmpstr(one_p, ==, "1024.00 TiB");

	g_free(zero);
	g_free(ten_b);
	g_free(max_b);
	g_free(one_k);
	g_free(one_point_five_k);
	g_free(one_m);
	g_free(one_point_nine_nine_m);
	g_free(one_g);
	g_free(one_t);
	g_free(one_p);
}

static void test_strtrunc_dep_desc(void)
{
	gchar *pkg_empty = strtrunc_dep_desc("");
	gchar *pkg_simple = strtrunc_dep_desc("pacfinder");
	gchar *pkg_version = strtrunc_dep_desc("pacfinder>=1.1");
	gchar *pkg_verepoch = strtrunc_dep_desc("pacfinder=1:1.1");
	gchar *pkg_desc = strtrunc_dep_desc("pacfinder: Package explorer");
	gchar *pkg_desc_colon = strtrunc_dep_desc("pacfinder: Package explorer: Arch");
	gchar *pkg_desc_epoch = strtrunc_dep_desc("pacfinder=1:1.1: Package explorer");
	gchar *pkg_desc_epoch_colon = strtrunc_dep_desc("pacfinder=1:1.1: Package explorer: Arch");
	gchar *prov_version = strtrunc_dep_desc("libalpm.so>=13");

	g_assert_cmpstr(pkg_empty, ==, "");
	g_assert_cmpstr(pkg_simple, ==, "pacfinder");
	g_assert_cmpstr(pkg_version, ==, "pacfinder>=1.1");
	g_assert_cmpstr(pkg_verepoch, ==, "pacfinder=1:1.1");
	g_assert_cmpstr(pkg_desc, ==, "pacfinder");
	g_assert_cmpstr(pkg_desc_colon, ==, "pacfinder");
	g_assert_cmpstr(pkg_desc_epoch, ==, "pacfinder=1:1.1");
	g_assert_cmpstr(pkg_desc_epoch_colon, ==, "pacfinder=1:1.1");
	g_assert_cmpstr(prov_version, ==, "libalpm.so>=13");

	g_free(pkg_empty);
	g_free(pkg_simple);
	g_free(pkg_version);
	g_free(pkg_verepoch);
	g_free(pkg_desc);
	g_free(pkg_desc_colon);
	g_free(pkg_desc_epoch);
	g_free(pkg_desc_epoch_colon);
	g_free(prov_version);
}

void test_util(void)
{
	g_test_add_func("/util/list_to_string", test_list_to_string);
	g_test_add_func("/util/human_readable_size", test_human_readable_size);
	g_test_add_func("/util/strtrunc_dep_desc", test_strtrunc_dep_desc);
}
