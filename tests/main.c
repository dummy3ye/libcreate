/*
 * main.c - Test runner and minimal harness for libcreate
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "create.h"
#include "harness.h"

/* ---- Test counters ---- */
int g_pass;
int g_fail;
int g_skip;

/* ---- Temporary directory management ---- */

static char g_tmpdir[256];

static void setup_tmpdir(void)
{
    const char *base = getenv("TMPDIR");
    if (!base || *base == '\0')
        base = "/tmp";
    snprintf(g_tmpdir, sizeof(g_tmpdir), "%s/libcreate_test_XXXXXX", base);
    if (mkdtemp(g_tmpdir) == NULL) {
        perror("mkdtemp");
        exit(1);
    }
}

/* Recursively remove the temp directory */
static void cleanup_tmpdir(void)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", g_tmpdir);
    (void)system(cmd);
}

const char *get_tmpdir(void)
{
    return g_tmpdir;
}

/* ---- Check if filesystem honors permission bits ---- */
static int g_fs_honors_perms = 1;

static void probe_filesystem_perms(void)
{
    char probe[512];
    snprintf(probe, sizeof(probe), "%s/.perm_probe", g_tmpdir);

    int fd = open(probe, O_WRONLY | O_CREAT | O_TRUNC, 0640);
    if (fd < 0) {
        g_fs_honors_perms = 0;
        return;
    }
    close(fd);

    struct stat st;
    if (stat(probe, &st) != 0) {
        g_fs_honors_perms = 0;
    } else {
        g_fs_honors_perms = ((st.st_mode & 0777) == 0640);
    }
    unlink(probe);

    if (!g_fs_honors_perms) {
        fprintf(stderr, "  NOTE: filesystem does not honor permission bits; "
                        "permission tests will be skipped.\n");
    }
}

int fs_honors_perms(void)
{
    return g_fs_honors_perms;
}

/* ---- Version consistency test ---- */

static void test_version_consistency(void)
{
    printf("  version consistency...\n");

#ifdef VERSION_FILE
    FILE *fp = fopen(VERSION_FILE, "r");
    TEST_ASSERT_MSG(fp != NULL, "could not open VERSION file");
    if (!fp)
        return;

    char buf[64];
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        TEST_ASSERT_MSG(0, "VERSION file is empty");
        fclose(fp);
        return;
    }
    fclose(fp);

    /* Strip trailing newline */
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0';

    TEST_ASSERT_MSG(strcmp(buf, LIBCREATE_VERSION) == 0,
                    "VERSION file does not match LIBCREATE_VERSION macro");
#else
    TEST_SKIP("VERSION_FILE not defined at compile time");
#endif
}

/* ---- Entry points from other test files ---- */
void test_basic(void);
void test_errors(void);
void test_parity(void);

/* ---- Main ---- */
int main(void)
{
    /* Set umask to 0 so permission assertions are deterministic */
    umask(0);

    setup_tmpdir();
    probe_filesystem_perms();

    printf("Running libcreate tests in %s\n\n", g_tmpdir);

    printf("[version]\n");
    test_version_consistency();

    printf("[basic]\n");
    test_basic();

    printf("[errors]\n");
    test_errors();

    printf("[parity]\n");
    test_parity();

    cleanup_tmpdir();

    printf("\n---\n%d passed, %d failed, %d skipped\n",
           g_pass, g_fail, g_skip);

    return g_fail > 0 ? 1 : 0;
}
