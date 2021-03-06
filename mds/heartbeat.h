/*
 * vim: ts=8:sw=8:tw=79:noet
 * 
 * Copyright 2012 the Redfish authors
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

#ifndef REDFISH_MDS_HEARTBEAT_DOT_H
#define REDFISH_MDS_HEARTBEAT_DOT_H

struct redfish_thread;

/** Runs the MDS heartbeat thread
 *
 * @param rt		The Redfish thread object
 *
 * @return		(never returns)
 */
extern int mds_send_hb_thread(struct redfish_thread *rt);

#endif
