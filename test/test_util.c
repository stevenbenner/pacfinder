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

#include <glib.h>
#include <math.h>

#include "util.h"

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
	g_assert_cmpstr(one_point_nine_nine_m, ==, "1.99 MiB");
	g_assert_cmpstr(one_m, ==, "1.00 MiB");
	g_assert_cmpstr(one_g, ==, "1.00 GiB");
	g_assert_cmpstr(one_t, ==, "1.00 TiB");
	g_assert_cmpstr(one_p, ==, "1024.00 TiB");

	g_free(zero);
	g_free(ten_b);
	g_free(max_b);
	g_free(one_k);
	g_free(one_point_five_k);
	g_free(one_point_nine_nine_m);
	g_free(one_m);
	g_free(one_g);
	g_free(one_t);
	g_free(one_p);
}

void test_util(void)
{
	g_test_add_func("/util/human_readable_size", test_human_readable_size);
}
