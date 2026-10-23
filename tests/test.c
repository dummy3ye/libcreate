/*
 * test.c - Basic functionality tests for create()
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "create.h"
#include "harness.h"

static void make_path(char *buf, size_t sz, const char *name)
{
    snprintf(buf, sz, "%s/%s", get_tmpdir(), name);
}

void test_basic(void)
{
    char path[512];
    struct stat st;

    /* 1. Creating a new file succeeds and returns fd >= 0 */
    printf("  new file creation...\n");
    make_path(path, sizeof(path), "basic_new");
    int fd = create(path, 0644);
    TEST_ASSERT(fd >= 0);
    if (fd >= 0)
        close(fd);
    TEST_ASSERT(stat(path, &st) == 0);

    /* 2. Newly created file has exactly the requested permissions (umask 0) */
    printf("  permissions (umask 0)...\n");
    if (fs_honors_perms()) {
        make_path(path, sizeof(path), "basic_perms");
        fd = create(path, 0751);
        TEST_ASSERT(fd >= 0);
        if (fd >= 0)
            close(fd);
        TEST_ASSERT(stat(path, &st) == 0);
        TEST_ASSERT_MSG((st.st_mode & 0777) == 0751, "permissions should be exactly 0751");
    } else {
        TEST_SKIP("filesystem does not honor mode bits");
    }

    /* 3. With non-zero umask, permissions are masked */
    printf("  permissions (umask 022)...\n");
    if (fs_honors_perms()) {
        mode_t old = umask(022);
        make_path(path, sizeof(path), "basic_umask");
        fd = create(path, 0666);
        TEST_ASSERT(fd >= 0);
        if (fd >= 0)
            close(fd);
        TEST_ASSERT(stat(path, &st) == 0);
        TEST_ASSERT_MSG((st.st_mode & 0777) == 0644, "permissions should be 0666 & ~022 = 0644");
        umask(old);
    } else {
        TEST_SKIP("filesystem does not honor mode bits");
    }

    /* 4. Writing through the returned descriptor works */
    printf("  write data...\n");
    make_path(path, sizeof(path), "basic_write");
    fd = create(path, 0644);
    TEST_ASSERT(fd >= 0);
    if (fd >= 0) {
        const char *msg = "hello libcreate\n";
        ssize_t n = write(fd, msg, strlen(msg));
        TEST_ASSERT(n == (ssize_t)strlen(msg));
        close(fd);

        /* Read it back */
        fd = open(path, O_RDONLY);
        TEST_ASSERT(fd >= 0);
        if (fd >= 0) {
            char buf[64];
            ssize_t r = read(fd, buf, sizeof(buf) - 1);
            TEST_ASSERT(r == (ssize_t)strlen(msg));
            if (r > 0) {
                buf[r] = '\0';
                TEST_ASSERT(strcmp(buf, msg) == 0);
            }
            close(fd);
        }
    }

    /* 5. Calling create() on an existing non-empty file truncates it */
    printf("  truncation...\n");
    make_path(path, sizeof(path), "basic_trunc");
    fd = create(path, 0644);
    TEST_ASSERT(fd >= 0);
    if (fd >= 0) {
        (void)write(fd, "some data here", 14);
        close(fd);
    }
    /* Truncate by calling create() again */
    fd = create(path, 0644);
    TEST_ASSERT(fd >= 0);
    if (fd >= 0)
        close(fd);
    TEST_ASSERT(stat(path, &st) == 0);
    TEST_ASSERT_MSG(st.st_size == 0, "file should be truncated to 0 bytes");

    /* 6. create() on existing file does NOT change its permissions */
    printf("  no permission change on existing...\n");
    if (fs_honors_perms()) {
        make_path(path, sizeof(path), "basic_nochmod");
        fd = create(path, 0600);
        TEST_ASSERT(fd >= 0);
        if (fd >= 0)
            close(fd);
        /* Call again with different mode */
        fd = create(path, 0777);
        TEST_ASSERT(fd >= 0);
        if (fd >= 0)
            close(fd);
        TEST_ASSERT(stat(path, &st) == 0);
        TEST_ASSERT_MSG((st.st_mode & 0777) == 0600,
                        "permissions should remain 0600 despite mode 0777");
    } else {
        TEST_SKIP("filesystem does not honor mode bits");
    }

    /* 7. Returned descriptor is write-only: read() fails with EBADF */
    printf("  write-only descriptor...\n");
    make_path(path, sizeof(path), "basic_rdonly");
    fd = create(path, 0644);
    TEST_ASSERT(fd >= 0);
    if (fd >= 0) {
        char buf[16];
        errno = 0;
        ssize_t r = read(fd, buf, sizeof(buf));
        TEST_ASSERT(r == -1);
        TEST_ASSERT(errno == EBADF);
        close(fd);
    }
}
