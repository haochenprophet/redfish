/*
 * Copyright 2011-2012 the Redfish authors
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

#include "core/glitch_log.h"
#include "common/config/logc.h"
#include "util/simple_io.h"
#include "util/tempfile.h"
#include "util/test.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define TEST_STR "I can see my house from here!\n"
#define TEST_STR2 "Second string!\n"

static int test_stderr_output(const char *tempdir)
{
	char buf[4096];
	int fd, ret;
	char tempfile[PATH_MAX];

	/* Redirect stderr to a file in our temporary directory. */
	snprintf(tempfile, sizeof(tempfile), "%s/stderr", tempdir);
	fd = open(tempfile, O_CREAT | O_TRUNC | O_WRONLY, 0600);
	if (fd == -1) {
		int err = errno;
		fprintf(stderr, "open(%s) err = %d\n", tempfile, err);
		return -err;
	}
	ret = dup2(fd, STDERR_FILENO);
	if (ret == -1) {
		int err = errno;
		fprintf(stderr, "dup2 err = %d\n", err);
		return -err;
	}
	glitch_log(TEST_STR);
	ret = simple_io_read_whole_file_zt(tempfile, buf, sizeof(buf));
	if (ret < 0) {
		fprintf(stderr, "simple_io_read_whole_file_zt(%s) failed: "
			"error %d\n", tempfile, ret);
		return ret;
	}
	if (strcmp(buf, TEST_STR) != 0) {
		fprintf(stderr, "read '%s'; expected to find '" TEST_STR
			"' in it.\n", buf);
		return -EDOM;
	}
	return 0;
}

static int test_log_output(const char *log)
{
	char expected_buf[4096];
	char buf[4096];

	glitch_log(TEST_STR2);

	/* Check the log file that should be written */
	EXPECT_POSITIVE(simple_io_read_whole_file_zt(log, buf, sizeof(buf)));
	snprintf(expected_buf, sizeof(expected_buf), "%s%s",
			TEST_STR, TEST_STR2);
	if (strcmp(buf, expected_buf) != 0) {
		fprintf(stderr, "read '%s'; expected '%s'\n", buf,
				expected_buf);
		return -EDOM;
	}
	return 0;
}

int main(void)
{
	char tempdir[PATH_MAX];
	char glitch_log_path[PATH_MAX];
	struct logc lc;

	EXPECT_ZERO(get_tempdir(tempdir, PATH_MAX, 0700));
	EXPECT_ZERO(register_tempdir_for_cleanup(tempdir));
	snprintf(glitch_log_path, sizeof(glitch_log_path),
			"%s/glitch_log.txt", tempdir);
	memset(&lc, 0, sizeof(struct logc));
	lc.glitch_log_path = glitch_log_path;
	EXPECT_ZERO(test_stderr_output(tempdir));
	configure_glitch_log(&lc);
	EXPECT_ZERO(test_log_output(lc.glitch_log_path));
	return EXIT_SUCCESS;
}
