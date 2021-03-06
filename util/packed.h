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

#ifndef REDFISH_UTIL_PACKED_H
#define REDFISH_UTIL_PACKED_H

#include <stdint.h> /* for uint32_t */

#include <unistd.h> /* for size_t */

extern uint8_t unpack_from_8(const void *v);
extern void pack_to_8(void *v, uint8_t u);

/**************** pack to big-endian byte order ********************/
extern uint16_t unpack_from_be16(const void *v);
extern void pack_to_be16(void *v, uint16_t u);

extern uint32_t unpack_from_be32(const void *v);
extern void pack_to_be32(void *v, uint32_t u);

extern uint64_t unpack_from_be64(const void *v);
extern void pack_to_be64(void *v, uint64_t u);

/**************** pack to host byte order ********************/
extern uint16_t unpack_from_h16(const void *v);
extern void pack_to_h16(void *v, uint16_t u);

extern uint32_t unpack_from_h32(const void *v);
extern void pack_to_h32(void *v, uint32_t u);

extern uint64_t unpack_from_h64(const void *v);
extern void pack_to_h64(void *v, uint64_t u);

extern void* unpack_from_hptr(const void *v);
extern void pack_to_hptr(void *v, void *ptr);

/** Retrieve a packed NULL-terminated string
 *
 * @param v		Pointer to the data
 * @param off		(inout) offset to start at.  When the function finishes,
 *			will be the next offset to pull things from.
 * @param len		Total length of the data
 * @param out		(out-param) where to put the string
 * @param out_len	length of the out buffer
 *
 * @return		0 on success.
 * 			-ENAMETOOLONG if the string we found was too long to fit
 * 			in the supplied buffer.
 * 			-EINVAL if we couldn't find a NULL before reaching the
 * 			end of the buffer.
 */
extern int unpack_str(const void *v, uint32_t *off, uint32_t len,
			char *out, size_t out_len);

/** Create a packed NULL-terminated string
 *
 * @param v		Pointer to the data
 * @param off		(inout) offset to start at.  When the function finishes,
 *			will be the next offset to push things to.
 * @param len		Total available length in the data buffer
 * @param str		The string to add
 *
 * @return		0 on success.
 * 			-ENAMETOOLONG if there wasn't enough memory to add
 * 			the string to the supplied buffer.
 */
extern int pack_str(void *v, uint32_t *off, uint32_t len, const char *str);

/** Copy a packed NULL-terminated string from one buffer to another
 *
 * @param ov		Pointer to the output data
 * @param ooff		(inout) offset to start at.  When the function finishes,
 *			will be the next offset to push things to.
 * @param olen		Total available length in the output buffer
 * @param iv		Pointer to the input data
 * @param ioff		(inout) offset to start at.  When the function finishes,
 *			will be the next offset to read things from.
 * @param ilen		Total available length in the input buffer
 *
 * @return		0 on success.
 * 			-ENAMETOOLONG if there wasn't enough memory to add
 * 			the string to the supplied buffer.
 */
extern int repack_str(void *ov, uint32_t *ooff, uint32_t olen,
			const void *iv, uint32_t *ioff, uint32_t ilen);

#endif
