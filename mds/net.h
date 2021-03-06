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

#ifndef REDFISH_MDS_NET_DOT_H
#define REDFISH_MDS_NET_DOT_H

#include "common/entity_type.h"

#include <stdint.h> /* for uint16_t, etc. */

struct fast_log_buf;
struct mdsc;
struct unitaryc;

/** Initialize mds networking stuff
 *
 * @param fb		The fast_log_buf to use
 * @param conf		The unitary Redfish configuration
 * @param mconf		The MDS configuration
 * @param mid		The MDS server ID of this metadata server
 */
void mds_net_init(struct fast_log_buf *fb, struct unitaryc *conf,
		struct mdsc *mconf, uint16_t mid);

/** Runs the main metadata server loop
 *
 * @return	0 on successful exit; error code otherwise
 */
int mds_main_loop(void);

#endif
