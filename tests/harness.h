/*
 * harness.h - Shared test macros and declarations
 */

#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include <stdio.h>

/* Counters (defined in main.c) */
extern int g_pass;
extern int g_fail;
extern int g_skip;

#define TEST_ASSERT(expr)                                                                          \
    do {                                                                                           \
        if (expr) {                                                                                \
            g_pass++;                                                                              \
        } else {                                                                                   \
            g_fail++;                                                                              \
            fprintf(stderr, "  FAIL: %s:%d: %s\n", __FILE__, __LINE__, #expr);                     \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_MSG(expr, msg)                                                                 \
    do {                                                                                           \
        if (expr) {                                                                                \
            g_pass++;                                                                              \
        } else {                                                                                   \
            g_fail++;                                                                              \
            fprintf(stderr, "  FAIL: %s:%d: %s (%s)\n", __FILE__, __LINE__, #expr, msg);           \
        }                                                                                          \
    } while (0)

#define TEST_SKIP(msg)                                                                             \
    do {                                                                                           \
        g_skip++;                                                                                  \
        fprintf(stderr, "  SKIP: %s\n", msg);                                                      \
    } while (0)

/* Helpers (defined in main.c) */
const char *get_tmpdir(void);
int fs_honors_perms(void);

#endif /* TEST_HARNESS_H */
