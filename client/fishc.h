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

#ifndef REDFISH_CLIENT_FISHC_DOT_H
#define REDFISH_CLIENT_FISHC_DOT_H

#include <stdint.h> /* for int64_t */

#include <unistd.h> /* for size_t */

#define REDFISH_INVAL_MODE 01000

/** Represents a Redfish client. Generally you will have one of these for each
 * process that is accessing the filesystem.
 *
 * All operations on a Redfish Client are thread-safe, except for
 * redfish_release_client and redfish_disconnect_and_release.
 */
struct redfish_client;

struct redfish_version
{
	uint32_t major;
	uint32_t minor;
	uint32_t patchlevel;
};

/** Type of a Redfish logging callback
 *
 * @param log_ctx		The client-supplied logging context
 * @param str			The string to log
 */
typedef void (*redfish_log_fn_t)(void *log_ctx, const char *str);

/** A logging callback which discards logs
 *
 * @param log_ctx		The client-supplied logging context (unused)
 * @param str			The string to log (unused)
 */
extern void redfish_log_to_dev_null(void *log_ctx, const char *str);

/** A logging callback which logs to stderr
 *
 * @param log_ctx		The client-supplied logging context (unused)
 * @param str			The string to log
 */
extern void redfish_log_to_stderr(void *log_ctx, const char *str);

/** Represents the location of a metadata server */
struct redfish_mds_locator
{
	char *host;
	int port;
};

/** Represents an open redfish file.
 *
 * All operations on a Redfish File are thread-safe, except for
 * redfish_free_file and redfish_close_and_free.
 */
struct redfish_file;

/** Represents the status of a Redfish file. */
struct redfish_stat
{
	int64_t length;
	short is_dir;
	short repl;
	int block_sz;
	int64_t mtime;
	int64_t atime;
	uint64_t nid;
	short mode;
	char *owner;
	char *group;
};

/** Represents an entry in a Redfish directory */
struct redfish_dir_entry
{
	char *name;
	struct redfish_stat stat;
};

struct redfish_block_host
{
	int port;
	char *hostname;
};

struct redfish_block_loc
{
	int64_t start;
	int64_t len;
	int nhosts;
	struct redfish_block_host hosts[0];
};

/** Get the version of the redfish client library
 *
 * @return		The redfish version
 */
struct redfish_version redfish_get_version(void);

/** Get the current redfish version as a string
 *
 * @return		a statically allocated string
 */
const char* redfish_get_version_str(void);

/** Initialize a Redfish client instance.
 *
 * @param conf_path	Path to a Redfish configuration file
 * @param user		The user to connect as
 * @param log_cb	The logging callback to use, or NULL if you do not want
 *			logging.  This function may be called concurrently by
 *			multiple threads.  If you require locking, you are
 *			responsible for providing it.
 * @param log_ctx	Context pointer that will be passed to log_cb
 * @param err		(out param) error buffer.  If there is an error, an
 *			error message will be placed here.  If there is not an
 *			error, this buffer will NOT be modified or zeroed.
 * @param err_len	length of error buffer.
 *
 * @return		The redfish client on success; NULL otherwise.
 *			On error, an error message will be placed into the err
 *			buffer.
 */
extern struct redfish_client *redfish_connect(const char *conf_path,
	const char *user, redfish_log_fn_t log_cb, void *log_ctx,
	char *err, size_t err_len);

/** Set the primary group for a user.
 * Redfish implements the traditional UNIX permissions model, wherein the
 * primary group of a user determines the group in which new files created by
 * that user will reside.
 * After you change the primary group of a user, he will continue to be a member
 * of the old primary group.  However, you can use
 * redfish_remove_user_from_group to remove him from that group.
 *
 * Only superusers can modify users other than themselves.
 *
 * @param cli		The Redfish client
 * @param group		The group name
 * @param user		The user name
 *
 * @return		0 on success; -EEXIST if the user is already a member
 *			of the group.
 */
extern int redfish_set_primary_user_group(struct redfish_client *cli,
		const char *user, const char *group);

/** Remove a user from a group
 * The user will be removed from the designated group.  If the group was the
 * user's primary group, his new primary group will be the same as his
 * username.
 *
 * Only superusers can modify users other than themselves.
 *
 * @param cli		The Redfish client
 * @param group		The group name
 * @param user		The user name
 *
 * @return		0 on success; -EEXIST if the user is already a member
 *			of the group.
 */
extern int redfish_add_user_to_group(struct redfish_client *cli,
		const char *user, const char *group);

/** Remove a user from a group
 * The user will be removed from the designated group.  If the group was the
 * user's primary group, his new primary group will be the same as his
 * username.  A user is always a member of the group of the same name.
 *
 * Only superusers can modify users other than themselves.
 *
 * @param cli		The Redfish client
 * @param group		The group name
 * @param user		The user name
 *
 * @return		0 on success; -ENOENT if the user is not currently known
 *			to be a member of the group.
 */
extern int redfish_remove_user_from_group(struct redfish_client *cli,
		const char *user, const char *group);

/** Get information about a user
 * Get information about a user.  The information will be given in the form of a
 * JSON string.
 *
 * The current format is:
 * {
 *	"user_name" : "<user_name>",
 *	"primary_group" : "<group_name>",
 *	"groups" : [ "<group_name>", ... ]
 * }
 *
 * New fields may be added in the future.
 *
 * @param cli		The Redfish client
 * @param group		The user name
 * @param buf		Buffer size
 * @param buf_len	Buffer length
 *
 * @return		0 on success
 *			-EMSGSIZE if the user-provided buffer is too short.
 *			Other error codes as appropriate.
 */
extern int redfish_get_user_info(struct redfish_client *cli,
		const char *user, char *buf, size_t buf_len);

/** Create a file in Redfish
 *
 * @param cli		The Redfish client
 * @param path		The file path
 * @param mode		The file mode
 * @param bufsz		The buffer size to use for the new file.
 *			0 means to use the default buffer size
 * @param repl		The number of replicas to use.
 *			0 means to use the default number of replicas
 * @param blocksz	The size of the blocks to use
 *			0 means to use the default block size
 * @param ofe		(out-parameter) the Redfish file
 *
 * @return		0 on success; error code otherwise
 *			On success, *ofe will contain a valid redfish file.
 */
int redfish_create(struct redfish_client *cli, const char *path,
	int mode, uint64_t bufsz, int repl, uint32_t blocksz,
	struct redfish_file **ofe);

/** Open a Redfish file for reading
 *
 * TODO: make buffer size configurable here?
 *
 * @param cli		The Redfish client
 * @param path		The file path
 * @param ofe		(out-parameter) the Redfish file
 *
 * @return		0 on success; error code otherwise
 *			On success, *ofe will contain a valid redfish file.
 */
int redfish_open(struct redfish_client *cli, const char *path,
		struct redfish_file **ofe);

/** Create a regular directory or directories in Redfish.
 *
 * Similar to mkdir -p, we will create the relevant directory as well as any
 * ancestor directories.
 *
 * @param cli		the Redfish client to use
 * @param mode		The permission to use when creating the directories.
 *			If this is REDFISH_INVAL_MODE, the default mode will
 *			be used.
 * @param ofe		(out-parameter): the redfish file
 *
 * Returns: 0 on success; error code otherwise
 * On success, *cli will contain a valid redfish file.
 */
int redfish_mkdirs(struct redfish_client *cli, int mode, const char *path);

/** Get the block locations
 *
 * Get the block locations where a given file is being stored.
 *
 * @param cli		the Redfish client
 * @paths path		path
 * @param start		Start location in the file
 * @param len		Length of the region of the file to examine
 * @param blc		(out-parameter) will contain a NULL-terminated array of
 *			pointers to block locations on success.
 * @return		negative number on error; 0 on success
 */
int redfish_locate(struct redfish_client *cli, const char *path,
	int64_t start, int64_t len, struct redfish_block_loc ***blc);

/** Free the array of block locations
 *
 * @param blc		The array of block locations
 * @param nblc		Length of the array of block locations
 */
void redfish_free_block_locs(struct redfish_block_loc **blc, int nblc);

/** Given a path, returns file status information
 *
 * @param cli		the Redfish client
 * @paths path		path
 * @param osa		(out-parameter): file status
 *
 * @return		0 on success; error code otherwise
 */
int redfish_get_path_status(struct redfish_client *cli, const char *path,
				struct redfish_stat* osa);

/** Given an open file, returns file status information
 *
 * This method will always go out to the MDSes, to get the latest status
 * information.  If the file is open for write, however, we will return the true
 * length, which may be longer than what the MDS thinks it is.
 *
 * This method is for the benefit of FUSE, mostly.  Hadoop doesn't really use
 * it.
 *
 * @param ofe		the Redfish file
 * @param osa		(out-parameter): file status
 *
 * @return		0 on success; error code otherwise
 */
int redfish_get_file_status(struct redfish_file *ofe,
		struct redfish_stat* osa);

/** Frees the status data returned by redfish_get_path_status
 *
 * @param osa		The file status
 */
void redfish_free_path_status(struct redfish_stat* osa);

/** Given a directory name, return a list of status objects corresponding
 * to the objects in that directory.
 * TODO: add some kind of filtering here?
 *
 * @param cli		the Redfish client
 * @param dir		the directory to get a listing from
 * @param osa		(out-parameter) an array of statuses
 *
 * @return		the number of status objects on success; a negative
 *			error code otherwise
 */
int redfish_list_directory(struct redfish_client *cli, const char *dir,
			      struct redfish_dir_entry **oda);

/** Frees the array of directory entries returned by redfish_list_directory
 *
 * @param oda		array of directory entries
 * @param noda		Length of oda
 */
void redfish_free_dir_entries(struct redfish_dir_entry* oda, int noda);

/** Changes the permission bits for a file or directory.
 *
 * @param cli		the Redfish client
 * @param path		the path
 * @param mode		the new permission bits
 *
 * @return		0 on success; error code otherwise
 */
int redfish_chmod(struct redfish_client *cli, const char *path, int mode);

/** Changes the owner and group of a file or directory.
 *
 * @param cli		the Redfish client
 * @param path		the path
 * @param owner		the new owner name, or NULL to leave owner unchanged
 * @param group		the new group name, or NULL to leave group unchanged
 *
 * @return		0 on success; error code otherwise
 */
int redfish_chown(struct redfish_client *cli, const char *path,
		  const char *owner, const char *group);

/** Changes the mtime and atime of a file
 *
 * @param cli		the Redfish client
 * @param path		the path
 * @param mtime		the new mtime, or -1 if the time should not be changed.
 * @param atime		the new atime, or -1 if the time should not be changed.
 *
 * @return		0 on success; error code otherwise
 */
int redfish_utimes(struct redfish_client *cli, const char *path,
		uint64_t mtime, uint64_t atime);

/** Disconnect a Redfish client instance
 *
 * Once a client instance is disconnected, no further operations can be
 * performed on it.  This function is thread-safe.
 *
 * There isn't anything you can do with a disconnected client except release it
 * using redfish_release_client.
 *
 * @param cli		the Redfish client to disconnect
 */
void redfish_disconnect(struct redfish_client *cli);

/** Release the memory associated with a Redfish client
 *
 * This function will release the memory associated with a Redfish client.
 * The memory may not actually be freed until all of the redfish_file
 * structures created by this client have been freed with redfish_free_file.
 *
 * You must ensure that no other thread is using the Redfish client pointer
 * while you are releasing it, or afterwards.
 *
 * The pointers to redfish_file structures created by this client will continue
 * to be valid even after the client itself is released.  The only really useful
 * thing you can do with those pointers at that time is to call
 * redfish_free_file on them.  Please remember to do this.
 *
 * @param cli		the Redfish client to free
 */
void redfish_release_client(struct redfish_client *cli);

/** Disconnect and free a Redfish client.
 *
 * This function is NOT thread-safe.  See redfish_release_client for an
 * explanation.
 *
 * @param cli		the Redfish client to disconnect and free.
 */
void redfish_disconnect_and_release(struct redfish_client *cli);

/** Reads data from a redfish file
 *
 * @param ofe		the Redfish file
 * @param data		a buffer to read into
 * @param len		the maximum length of the data to read
 *
 * @return		the number of bytes read on success; a negative error
 *			code on failure.  0 indicates EOF.
 *			We won't return a short read unless the file itself is
 *			shorter than the requested length.
 */
int redfish_read(struct redfish_file *ofe, void *data, int len);

/** Returns the number of bytes that can be read from the Redfish file without
 * blocking.
 *
 * @param ofe		the Redfish file
 *
 * @return		the number of bytes available to read
 */
int32_t redfish_available(struct redfish_file *ofe);

/** Reads data from a redfish file
 *
 * @param ofe		the Redfish file
 * @param data		a buffer to read into
 * @param len		the maximum length of the data to read
 * @param off		offset to read data from
 *
 * @return		the number of bytes read on success; a negative error
 *			code on failure.
 */
int redfish_pread(struct redfish_file *ofe, void *data, int len, int64_t off);

/** Writes data to a redfish file
 *
 * @param ofe		the Redfish file
 * @param data		the data to write
 * @param len		the length of the data to write
 *
 * @return		0 on success; error code otherwise
 */
int redfish_write(struct redfish_file *ofe, const void *data, int len);

/** Set the current position in a file
 * This works only for files opened in read-only mode.
 *
 * @param ofe		the Redfish file
 * @param off		the desired new offset
 *
 * @return		0 on success; error code otherwise
 */
int redfish_fseek_abs(struct redfish_file *ofe, uint64_t off);

/** Set the current position in a file
 * This works only for files opened in read-only mode.
 *
 * @param ofe		the Redfish file
 * @param delta		the desired change in offset
 * @param out		(out param) the actual change in offset that was made
 *
 * @return		0 on success; error code otherwise
 */
int redfish_fseek_rel(struct redfish_file *ofe, int64_t delta, int64_t *out);

/** Get the current position in a file
 *
 * @param ofe		the Redfish file
 *
 * Returns the current position in the file
 */
int64_t redfish_ftell(struct redfish_file *ofe);

/** Make all the data in this file visible to readers who open the file after
 * this call to hflush. Data will be visible eventually to readers who already
 * have the file open, but do not reopen it.
 *
 * This is a blocking call. It will start the hflush operation and then block
 * until it completes. Among other things, this implies that the data is
 * replicated on all relevant OSDs.
 *
 * This call can be used only on files that have been opened in append mode.
 *
 * @param ofe		The redfish file
 *
 * @return		0 on success; error code otherwise
 */
int redfish_hflush(struct redfish_file *ofe);

/** Block until the data is really written to disk on by the chunkservers.
 *
 * This will actually block until the data has been persisted to disk.
 * Basically, fsync.
 *
 * @param ofe		The redfish file to sync
 *
 * @return		0 on success; error code otherwise
 */
int redfish_hsync(struct redfish_file *ofe);

/** Delete a file
 *
 * @param cli		the redfish client
 * @param path		the file to unlink
 *
 * @return		0 on success; error code otherwise
 */
int redfish_unlink(struct redfish_client *cli, const char *path);

/** Delete a directory if it's empty
 *
 * @param cli		the redfish client
 * @param path		the directory to remove
 *
 * @return		0 on success; error code otherwise
 */
int redfish_rmdir(struct redfish_client *cli, const char *path);

/** Delete a file or directory subtree
 *
 * Unlike redfish_rmdir, this will delete both directories and files.  It will
 * also recursively delete everything under a directory.
 * The recursive deletion is not guaranteed to be atomic.
 *
 * @param cli		the redfish client
 * @param path		the file or subtree to remove
 *
 * @return		0 on success; error code otherwise
 */
int redfish_unlink_tree(struct redfish_client *cli, const char *path);

/** Rename a file or directory
 *
 * @param src		the source path
 * @param dst		the destination path
 *
 * @return		0 on success; error code otherwise
 */
int redfish_rename(struct redfish_client *cli, const char *src, const char *dst);

/** Close a Redfish file.
 *
 * For files opened for writing or appending, this triggers any locally
 * buffered data to be written out to the metadata servers.
 *
 * This operation is thread-safe.
 *
 * @param ofe		the Redfish file
 *
 * @return		0 on success; error code if the buffered data could not
 *			be written out as expected.
 */
int redfish_close(struct redfish_file *ofe);

/** Freed the memory and internal state associated with a Redfish file.
 *
 * This operation does NOT properly close the file.  You may lose data if you
 * free a file opened for write before closing it.
 *
 * This operation is NOT thread-safe!  You must ensure that no other thread is
 * using the Redfish file while it is being freed.  After the file is freed, the
 * pointer becomes invalid and must never be used again.
 *
 * @param ofe		the Redfish file
 */
void redfish_free_file(struct redfish_file *ofe);

/** Close and free a Redfish file.
 *
 * This is a convenience method.  It is equivalent to redfish_close followed by
 * redfish_free_file.  Like redfish_free_file, it is NOT thread-safe.
 *
 * @param ofe		the Redfish file
 *
 * @return		the return code of redfish_close
 */
int redfish_close_and_free(struct redfish_file *ofe);

/* TODO: implement something like statvfs */
/* TODO: implement something like truncate */
/* TODO: implement open-for-append */
/* TODO: implement set replication */

#endif
