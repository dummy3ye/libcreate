/*
 * parity.c - Verify create() matches creat() exactly
 *
 * For each scenario, run both create() and creat() against fresh paths
 * and assert that the results agree: return-value category, errno,
 * resulting file size, and resulting file mode.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "create.h"
#include "harness.h"

/*
 * Compare a create() call vs. a creat() call on two separate paths
 * for the same scenario.
 */
static void compare(const char *label,
                    const char *path_create, const char *path_creat,
                    mode_t mode)
{
    struct stat st_c, st_r;
    int has_stat_c, has_stat_r;

    /* Run create() */
    errno = 0;
    int fd_c = create(path_create, mode);
    int err_c = errno;
    if (fd_c >= 0)
        close(fd_c);
    has_stat_c = (stat(path_create, &st_c) == 0);

    /* Run creat() */
    errno = 0;
    int fd_r = creat(path_creat, mode);
    int err_r = errno;
    if (fd_r >= 0)
        close(fd_r);
    has_stat_r = (stat(path_creat, &st_r) == 0);

    /* Compare return-value category */
    int ok_c = (fd_c >= 0);
    int ok_r = (fd_r >= 0);
    if (ok_c != ok_r) {
        fprintf(stderr, "  FAIL parity [%s]: create()=%d creat()=%d\n",
                label, fd_c, fd_r);
        g_fail++;
        return;
    }
    g_pass++;

    if (!ok_c) {
        /* Both failed — compare errno */
        TEST_ASSERT_MSG(err_c == err_r,
                        "errno mismatch between create() and creat()");
        return;
    }

    /* Both succeeded — compare file properties */
    TEST_ASSERT_MSG(has_stat_c == has_stat_r, "stat availability mismatch");

    if (has_stat_c && has_stat_r) {
        TEST_ASSERT_MSG(st_c.st_size == st_r.st_size, "size mismatch");
        if (fs_honors_perms()) {
            TEST_ASSERT_MSG((st_c.st_mode & 0777) == (st_r.st_mode & 0777),
                            "mode mismatch");
        }
    }
}

void test_parity(void)
{
    char p1[512], p2[512];

    /* Scenario 1: new file */
    printf("  parity: new file...\n");
    snprintf(p1, sizeof(p1), "%s/par_new_create", get_tmpdir());
    snprintf(p2, sizeof(p2), "%s/par_new_creat", get_tmpdir());
    compare("new file", p1, p2, 0644);

    /* Scenario 2: existing file (should truncate) */
    printf("  parity: existing file...\n");
    snprintf(p1, sizeof(p1), "%s/par_exist_create", get_tmpdir());
    snprintf(p2, sizeof(p2), "%s/par_exist_creat", get_tmpdir());
    /* Pre-create both with some data */
    {
        int fd;
        fd = open(p1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd >= 0) { (void)write(fd, "data", 4); close(fd); }
        fd = open(p2, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd >= 0) { (void)write(fd, "data", 4); close(fd); }
    }
    compare("existing file", p1, p2, 0644);

    /* Scenario 3: nonexistent directory (should fail with ENOENT) */
    printf("  parity: nonexistent dir...\n");
    snprintf(p1, sizeof(p1), "%s/nodir1/file", get_tmpdir());
    snprintf(p2, sizeof(p2), "%s/nodir2/file", get_tmpdir());
    compare("nonexistent dir", p1, p2, 0644);

    /* Scenario 4: directory target (should fail with EISDIR) */
    printf("  parity: directory target...\n");
    snprintf(p1, sizeof(p1), "%s/par_dir_create", get_tmpdir());
    snprintf(p2, sizeof(p2), "%s/par_dir_creat", get_tmpdir());
    mkdir(p1, 0755);
    mkdir(p2, 0755);
    compare("directory target", p1, p2, 0644);
}
