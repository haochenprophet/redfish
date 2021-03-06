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

#include "client/fishc.h"
#include "tool/tool.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int fishtool_ping(struct fishtool_params *params)
{
	int ret = 0;
	struct redfish_client *cli;
	char err[512] = { 0 };
	size_t err_len = sizeof(err);

	cli = redfish_connect(params->cpath, params->user_name,
		redfish_log_to_stderr, NULL, err, err_len);
	if (err[0]) {
		fprintf(stderr, "redfish_connect: failed to connect: "
				"%s\n", err);
		ret = -EIO;
		goto done;
	}
	redfish_disconnect_and_release(cli);

done:
	return ret;
}

const char *fishtool_ping_usage[] = {
	"ping: connect to the metadata servers and then immediately close ",
	"the connection",
	NULL,
};

struct fishtool_act g_fishtool_ping = {
	.name = "ping",
	.fn = fishtool_ping,
	.getopt_str = "",
	.usage = fishtool_ping_usage,
};
