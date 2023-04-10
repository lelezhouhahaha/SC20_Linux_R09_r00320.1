/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "config.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "config-parser.h"

#include "shared/helpers.h"
#include "zunitc/zunitc.h"

struct fixture_data {
	const char *text;
	struct weston_config *config;
};

static struct weston_config *
load_config(const char *text)
{
	struct weston_config *config = NULL;
	int len = 0;
	int fd = -1;
	char file[] = "/tmp/weston-config-parser-test-XXXXXX";

	ZUC_ASSERTG_NOT_NULL(text, out);

	fd = mkstemp(file);
	ZUC_ASSERTG_NE(-1, fd, out);

	len = write(fd, text, strlen(text));
	ZUC_ASSERTG_EQ((int)strlen(text), len, out_close);

	config = weston_config_parse(file);

out_close:
	close(fd);
	unlink(file);
out:
	return config;
}

static void *
setup_test_config(void *data)
{
	struct weston_config *config = load_config(data);
	ZUC_ASSERTG_NOT_NULL(config, out);

out:
	return config;
}

static void *
setup_test_config_failing(void *data)
{
	struct weston_config *config = load_config(data);
	ZUC_ASSERTG_NULL(config, err_free);

	return config;
err_free:
	weston_config_destroy(config);
	return NULL;
}

static void
cleanup_test_config(void *data)
{
	struct weston_config *config = data;
	ZUC_ASSERT_NOT_NULL(config);
	weston_config_destroy(config);
}

static struct zuc_fixture config_test_t0 = {
	.data = "# nothing in this file...\n",
	.set_up = setup_test_config,
	.tear_down = cleanup_test_config
};

static struct zuc_fixture config_test_t1 = {
	.data =
	"# comment line here...\n"
	"\n"
	"[foo]\n"
	"a=b\n"
	"name=  Roy Batty    \n"
	"\n"
	"\n"
	"[bar]\n"
	"# more comments\n"
	"number=5252\n"
	"flag=false\n"
	"\n"
	"[stuff]\n"
	"flag=     true \n"
	"\n"
	"[bucket]\n"
	"color=blue \n"
	"contents=live crabs\n"
	"pinchy=true\n"
	"\n"
	"[bucket]\n"
	"material=plastic \n"
	"color=red\n"
	"contents=sand\n",
	.set_up = setup_test_config,
	.tear_down = cleanup_test_config
};

static const char *section_names[] = {
	"foo", "bar", "stuff", "bucket", "bucket"
};

/*
 * Since these next few won't parse, we don't add the tear_down to
 * attempt cleanup.
 */

static struct zuc_fixture config_test_t2 = {
	.data =
	"# invalid section...\n"
	"[this bracket isn't closed\n",
	.set_up = setup_test_config_failing,
};

static struct zuc_fixture config_test_t3 = {
	.data =
	"# line without = ...\n"
	"[bambam]\n"
	"this line isn't any kind of valid\n",
	.set_up = setup_test_config_failing,
};

static struct zuc_fixture config_test_t4 = {
	.data =
	"# starting with = ...\n"
	"[bambam]\n"
	"=not valid at all\n",
	.set_up = setup_test_config_failing,
};

ZUC_TEST_F(config_test_t0, comment_only)
{
	struct weston_config *config = data;
	ZUC_ASSERT_NOT_NULL(config);
}

/** @todo individual t1 tests should have more descriptive names. */

ZUC_TEST_F(config_test_t1, test001)
{
	struct weston_config_section *section;
	struct weston_config *config = data;
	ZUC_ASSERT_NOT_NULL(config);
	section = weston_config_get_section(config,
					    "mollusc", NULL, NULL);
	ZUC_ASSERT_NULL(section);
}

ZUC_TEST_F(config_test_t1, test002)
{
	char *s;
	int r;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "foo", NULL, NULL);
	r = weston_config_section_get_string(section, "a", &s, NULL);

	ZUC_ASSERTG_EQ(0, r, out_free);
	ZUC_ASSERTG_STREQ("b", s, out_free);

out_free:
	free(s);
}

ZUC_TEST_F(config_test_t1, test003)
{
	char *s;
	int r;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "foo", NULL, NULL);
	r = weston_config_section_get_string(section, "b", &s, NULL);

	ZUC_ASSERT_EQ(-1, r);
	ZUC_ASSERT_EQ(ENOENT, errno);
	ZUC_ASSERT_NULL(s);
}

ZUC_TEST_F(config_test_t1, test004)
{
	char *s;
	int r;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "foo", NULL, NULL);
	r = weston_config_section_get_string(section, "name", &s, NULL);

	ZUC_ASSERTG_EQ(0, r, out_free);
	ZUC_ASSERTG_STREQ("Roy Batty", s, out_free);

out_free:
	free(s);
}

ZUC_TEST_F(config_test_t1, test005)
{
	char *s;
	int r;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "bar", NULL, NULL);
	r = weston_config_section_get_string(section, "a", &s, "boo");

	ZUC_ASSERTG_EQ(-1, r, out_free);
	ZUC_ASSERTG_EQ(ENOENT, errno, out_free);
	ZUC_ASSERTG_STREQ("boo", s, out_free);

out_free:
	free(s);
}

ZUC_TEST_F(config_test_t1, test006)
{
	int r;
	int32_t n;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "bar", NULL, NULL);
	r = weston_config_section_get_int(section, "number", &n, 600);

	ZUC_ASSERT_EQ(0, r);
	ZUC_ASSERT_EQ(5252, n);
}

ZUC_TEST_F(config_test_t1, test007)
{
	int r;
	int32_t n;
	struct weston_config_section *section;
	struct weston_config *config = data;;

	section = weston_config_get_section(config, "bar", NULL, NULL);
	r = weston_config_section_get_int(section, "+++", &n, 700);

	ZUC_ASSERT_EQ(-1, r);
	ZUC_ASSERT_EQ(ENOENT, errno);
	ZUC_ASSERT_EQ(700, n);
}

ZUC_TEST_F(config_test_t1, test008)
{
	int r;
	uint32_t u;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "bar", NULL, NULL);
	r = weston_config_section_get_uint(section, "number", &u, 600);
	ZUC_ASSERT_EQ(0, r);
	ZUC_ASSERT_EQ(5252, u);
}

ZUC_TEST_F(config_test_t1, test009)
{
	int r;
	uint32_t u;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "bar", NULL, NULL);
	r = weston_config_section_get_uint(section, "+++", &u, 600);
	ZUC_ASSERT_EQ(-1, r);
	ZUC_ASSERT_EQ(ENOENT, errno);
	ZUC_ASSERT_EQ(600, u);
}

ZUC_TEST_F(config_test_t1, test010)
{
	int r, b;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "bar", NULL, NULL);
	r = weston_config_section_get_bool(section, "flag", &b, 600);
	ZUC_ASSERT_EQ(0, r);
	ZUC_ASSERT_EQ(0, b);
}

ZUC_TEST_F(config_test_t1, test011)
{
	int r, b;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "stuff", NULL, NULL);
	r = weston_config_section_get_bool(section, "flag", &b, -1);
	ZUC_ASSERT_EQ(0, r);
	ZUC_ASSERT_EQ(1, b);
}

ZUC_TEST_F(config_test_t1, test012)
{
	int r, b;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "stuff", NULL, NULL);
	r = weston_config_section_get_bool(section, "flag", &b, -1);
	ZUC_ASSERT_EQ(0, r);
	ZUC_ASSERT_EQ(1, b);
}

ZUC_TEST_F(config_test_t1, test013)
{
	int r, b;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config, "stuff", NULL, NULL);
	r = weston_config_section_get_bool(section, "bonk", &b, -1);
	ZUC_ASSERT_EQ(-1, r);
	ZUC_ASSERT_EQ(ENOENT, errno);
	ZUC_ASSERT_EQ(-1, b);
}

ZUC_TEST_F(config_test_t1, test014)
{
	char *s;
	int r;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config,
					    "bucket", "color", "blue");
	r = weston_config_section_get_string(section, "contents", &s, NULL);

	ZUC_ASSERTG_EQ(0, r, out_free);
	ZUC_ASSERTG_STREQ("live crabs", s, out_free);

out_free:
	free(s);
}

ZUC_TEST_F(config_test_t1, test015)
{
	char *s;
	int r;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config,
					    "bucket", "color", "red");
	r = weston_config_section_get_string(section, "contents", &s, NULL);

	ZUC_ASSERTG_EQ(0, r, out_free);
	ZUC_ASSERTG_STREQ("sand", s, out_free);

out_free:
	free(s);
}

ZUC_TEST_F(config_test_t1, test016)
{
	char *s;
	int r;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = weston_config_get_section(config,
					    "bucket", "color", "pink");
	ZUC_ASSERT_NULL(section);
	r = weston_config_section_get_string(section, "contents", &s, "eels");

	ZUC_ASSERTG_EQ(-1, r, out_free);
	ZUC_ASSERTG_EQ(ENOENT, errno, out_free);
	ZUC_ASSERTG_STREQ("eels", s, out_free);

out_free:
	free(s);
}

ZUC_TEST_F(config_test_t1, test017)
{
	const char *name;
	int i;
	struct weston_config_section *section;
	struct weston_config *config = data;

	section = NULL;
	i = 0;
	while (weston_config_next_section(config, &section, &name))
		ZUC_ASSERT_STREQ(section_names[i++], name);

	ZUC_ASSERT_EQ(5, i);
}

ZUC_TEST_F(config_test_t2, doesnt_parse)
{
	struct weston_config *config = data;
	ZUC_ASSERT_NULL(config);
}

ZUC_TEST_F(config_test_t3, doesnt_parse)
{
	struct weston_config *config = data;
	ZUC_ASSERT_NULL(config);
}

ZUC_TEST_F(config_test_t4, doesnt_parse)
{
	struct weston_config *config = data;
	ZUC_ASSERT_NULL(config);
}

ZUC_TEST(config_test, destroy_null)
{
	weston_config_destroy(NULL);
	ZUC_ASSERT_EQ(0, weston_config_next_section(NULL, NULL, NULL));
}

ZUC_TEST(config_test, section_from_null)
{
	struct weston_config_section *section;
	section = weston_config_get_section(NULL, "bucket", NULL, NULL);
	ZUC_ASSERT_NULL(section);
}
