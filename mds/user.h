/*
 * The RedFish distributed filesystem
 *
 * Copyright (C) 2011 Colin Patrick McCabe <cmccabe@alumni.cmu.edu>
 *
 * This is licensed under the Apache License, Version 2.0.  See file COPYING.
 */

#ifndef REDFISH_MDS_USER_DOT_H
#define REDFISH_MDS_USER_DOT_H

#include <stdint.h> /* for uint32_t, etc. */

#include "util/tree.h" /* for RB_ENTRY */

struct user {
	RB_ENTRY(user) entry;
	/** user ID */
	uint32_t uid;
	/** user name */
	char name[RF_USER_MAX];
	/** primary group ID */
	uint32_t gid;
	/** number of secondary groups */
	uint32_t num_segid;
	/** secondary groups */
	uint32_t segid[0];
};

struct group {
	RB_ENTRY(group) entry;
	/** group ID */
	uint32_t gid;
	/** group name */
	char name[0];
};

/** Allocate an empty user data lookup cache
 *
 * @return		The user data, or an error pointer on failure
 */
extern struct udata *udata_alloc(void);

/** Free a user data lookup cache
 *
 * @param udata		The user data
 */
extern void udata_free(struct udata *udata);

/** Determine if a user is in a group
 *
 * @param user		The user
 * @param gid		The group id
 *
 * @return		0 if the user is not in the group; 1 otherwise
 */
extern int user_in_gid(const struct user *user, uint32_t gid);

/** Add a secondary group ID to a user
 *
 * @param user		The user
 * @param gid		The secondary group id
 *
 * @return		0 on success,
 *			-EEXIST if the user is already a member of the group,
 *			-ENOMEM on OOM
 */
extern int user_add_segid(struct user *user, uint32_t segid);

/** Get information about a user given his name
 *
 * @param udata		The user data
 * @param name		The user name
 *
 * @return		The user data, or an error pointer
 */
extern const struct user *udata_lookup_user(struct udata *udata,
		const char *name);

/** Get information about a group given its name
 *
 * @param udata		The user data
 * @param name		The group name
 *
 * @return		The group data, or an error pointer
 */
extern const struct group *udata_lookup_group(struct udata *udata,
		const char *name);

/** Add another user
 *
 * @param udata		The user data
 * @param name		The new user name
 * @param uid		The new user ID, or RF_INVAL_UID to use the next
 *			available ID
 * @param gid		The primary group ID of the new user
 *
 * @return		0 on success; error code otherwise
 */
extern int udata_add_user(struct udata *udata,
		const char *name, uint32_t uid, uint32_t gid);

/** Add another group
 *
 * @param udata		The user data
 * @param name		The new group name
 * @param uid		The new group ID, or RF_INVAL_GID to use the next
 *			available ID
 *
 * @return		0 on success; error code otherwise
 */
extern int udata_add_group(struct udata *udata,
		const char *name, uint32_t gid);

/** Pack user information to a byte buffer
 *
 * @param user		The user
 * @param buf		The buffer to pack to
 * @param off		(inout) offset in the buffer to pack to
 * @param max		size of the buffer to pack to
 *
 * @return		0 on success; error code otherwise
 */
extern int pack_user(const struct user *user, char *buf,
		uint32_t *off, uint32_t max);

/** Pack group information to a byte buffer
 *
 * @param group		The group
 * @param buf		The buffer to pack to
 * @param off		(inout) offset in the buffer to pack to
 * @param max		size of the buffer to pack to
 *
 * @return		0 on success; error code otherwise
 */
extern int pack_group(const struct group *group, char *buf,
		uint32_t *off, uint32_t max);

/** Pack userdata to a byte buffer
 *
 * @param udata		The userdata
 * @param buf		The buffer to pack to
 * @param off		(inout) offset in the buffer to pack to
 * @param max		size of the buffer to pack to
 *
 * @return		0 on success; error code otherwise
 */
extern int pack_udata(struct udata *udata, char *buf,
		uint32_t *off, uint32_t max);

/** Unpack user information from a byte buffer
 *
 * @param buf		The buffer to unpack from
 * @param off		(inout) offset in the buffer to unpack from
 * @param max		size of the buffer to unpack from
 *
 * @return		the dynamically allocated user data, or an error ptr
 */
extern struct user *unpack_user(char *buf, uint32_t *off, uint32_t max);

/** Unpack group information from a byte buffer
 *
 * @param buf		The buffer to unpack from
 * @param off		(inout) offset in the buffer to unpack from
 * @param max		size of the buffer to unpack from
 *
 * @return		the dynamically allocated group data, or an error ptr
 */
extern struct group *unpack_group(char *buf, uint32_t *off, uint32_t max);

/** Unpack userdata information from a byte buffer
 *
 * @param buf		The buffer to unpack from
 * @param off		(inout) offset in the buffer to unpack from
 * @param max		size of the buffer to unpack from
 *
 * @return		the dynamically allocated userdata, or an error ptr
 */
extern struct udata* unpack_udata(char *buf, uint32_t *off, uint32_t max);

#endif
