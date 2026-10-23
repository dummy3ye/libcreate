/*
 * create.h - Public header for libcreate
 *
 * libcreate provides create(), the function Ken Thompson said
 * creat() should have been named.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBCREATE_H
#define LIBCREATE_H

#include <sys/types.h>

/* Library version */
#define LIBCREATE_VERSION_MAJOR 1
#define LIBCREATE_VERSION_MINOR 0
#define LIBCREATE_VERSION_PATCH 0
#define LIBCREATE_VERSION       "1.0.0"

/* Symbol visibility */
#if defined(__GNUC__) && __GNUC__ >= 4
#define LIBCREATE_API __attribute__((visibility("default")))
#else
#define LIBCREATE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * create - Create or truncate a file.
 *
 * Equivalent to calling open(path, O_WRONLY | O_CREAT | O_TRUNC, mode).
 * This is the same behavior as the POSIX creat() function, but with the
 * correct spelling.
 *
 * @path:  Pathname of the file to create or truncate.  Must not be NULL;
 *         passing NULL is undefined behavior, as with creat().
 * @mode:  Permission bits for the new file (modified by the process umask).
 *         If the file already exists, mode is ignored and the existing
 *         permissions are preserved.
 *
 * On success, returns a non-negative file descriptor opened for writing
 * only.  Reading from the returned descriptor will fail with EBADF.
 *
 * On failure, returns -1 and sets errno.  Common errno values include:
 *   EACCES    - Permission denied.
 *   EISDIR    - Path refers to a directory.
 *   EMFILE    - Per-process file descriptor limit reached.
 *   ENAMETOOLONG - Path or a path component is too long.
 *   ENOENT    - A directory component of path does not exist.
 *   ENOSPC    - No space left on device.
 *   ENOTDIR   - A component of the path prefix is not a directory.
 *
 * Thread safety: same as open(2) — safe to call from multiple threads.
 */
LIBCREATE_API int create(const char *path, mode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* LIBCREATE_H */
