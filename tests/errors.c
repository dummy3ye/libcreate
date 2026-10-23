/*
 * errors.c - Error-path tests for create()
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "create.h"
#include "harness.h"

void test_errors(void)
{
    char path[512];
    int fd;

    /* 1. Path inside a nonexistent directory -> ENOENT */
    printf("  ENOENT (nonexistent dir)...\n");
    snprintf(path, sizeof(path), "%s/no_such_dir/file", get_tmpdir());
    errno = 0;
    fd = create(path, 0644);
    TEST_ASSERT(fd == -1);
    TEST_ASSERT(errno == ENOENT);

    /* 2. Empty-string path -> ENOENT */
    printf("  ENOENT (empty path)...\n");
    errno = 0;
    fd = create("", 0644);
    TEST_ASSERT(fd == -1);
    TEST_ASSERT(errno == ENOENT);

    /* 3. Parent component is a regular file -> ENOTDIR */
    printf("  ENOTDIR...\n");
    snprintf(path, sizeof(path), "%s/afile", get_tmpdir());
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0)
        close(fd);
    {
        char subpath[1024];
        snprintf(subpath, sizeof(subpath), "%s/child", path);
        errno = 0;
        fd = create(subpath, 0644);
        TEST_ASSERT(fd == -1);
        TEST_ASSERT(errno == ENOTDIR);
    }

    /* 4. Path is an existing directory -> EISDIR */
    printf("  EISDIR...\n");
    snprintf(path, sizeof(path), "%s/adir", get_tmpdir());
    mkdir(path, 0755);
    errno = 0;
    fd = create(path, 0644);
    TEST_ASSERT(fd == -1);
    TEST_ASSERT(errno == EISDIR);

    /* 5. No write permission on parent directory -> EACCES */
    printf("  EACCES...\n");
    if (geteuid() == 0) {
        TEST_SKIP("running as root — EACCES test skipped (root bypasses permissions)");
    } else {
        snprintf(path, sizeof(path), "%s/noaccess", get_tmpdir());
        mkdir(path, 0555);
        {
            char subpath[1024];
            snprintf(subpath, sizeof(subpath), "%s/file", path);
            errno = 0;
            fd = create(subpath, 0644);
            TEST_ASSERT(fd == -1);
            TEST_ASSERT(errno == EACCES);
        }
        /* Restore permissions so cleanup can remove it */
        chmod(path, 0755);
    }

    /* 6. Path component exceeding NAME_MAX -> ENAMETOOLONG */
    printf("  ENAMETOOLONG...\n");
    {
        /* Build a name longer than NAME_MAX (typically 255) */
        char longname[512];
        memset(longname, 'a', sizeof(longname) - 1);
        longname[sizeof(longname) - 1] = '\0';

        char longpath[1024];
        snprintf(longpath, sizeof(longpath), "%s/%s", get_tmpdir(), longname);
        errno = 0;
        fd = create(longpath, 0644);
        TEST_ASSERT(fd == -1);
        TEST_ASSERT(errno == ENAMETOOLONG);
    }
}
