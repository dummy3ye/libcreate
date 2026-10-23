/*
 * create.c - Implementation of create()
 *
 * "What's one of the things I'd do differently if I were redesigning
 * the UNIX system?  I'd spell creat with an 'e'." — Ken Thompson
 */

#include "create.h"
#include <fcntl.h>

int create(const char *path, mode_t mode)
{
    return open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}
