/*
 * hello.c - Minimal example of using libcreate
 *
 * Build with:
 *   cc -o hello hello.c $(pkg-config --cflags --libs libcreate)
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <create.h>

int main(int argc, char *argv[])
{
    const char *path = (argc > 1) ? argv[1] : "hello.txt";

    int fd = create(path, 0644);
    if (fd < 0) {
        perror("create");
        return 1;
    }

    const char *msg = "Hello from libcreate!\n";
    if (write(fd, msg, strlen(msg)) < 0) {
        perror("write");
        close(fd);
        return 1;
    }

    close(fd);
    printf("Wrote to %s\n", path);
    return 0;
}
