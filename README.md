# libcreate

A tiny C library that provides `create()` — identical to POSIX `creat()`, but with the missing **e**.

## Why?

Ken Thompson once said that if he could change one thing about UNIX, he'd spell `creat` with an 'e'. Decades later, here it is.

## Quick Start

```c
#include <create.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    int fd = create("greeting.txt", 0644);
    if (fd < 0) {
        perror("create");
        return 1;
    }

    const char *msg = "Hello, world!\n";
    write(fd, msg, strlen(msg));
    close(fd);
    return 0;
}
```

Compile with:

```sh
cc -o example example.c -lcreate
```

## Installation

### Build from source

```sh
make
sudo make install
```

### Custom prefix

```sh
make PREFIX=/opt/libcreate
sudo make PREFIX=/opt/libcreate install
```

### Staged install (for packaging)

```sh
make
make DESTDIR=/tmp/staging install
```

### pkg-config

After installation, use pkg-config to get the right flags:

```sh
cc -o myapp myapp.c $(pkg-config --cflags --libs libcreate)
```

### Vendoring (two files)

Copy `include/create.h` and `src/create.c` into your project and compile them directly:

```sh
cc -o myapp myapp.c create.c -Ipath/to/include
```

No special build flags are required.

## API Reference

### `int create(const char *path, mode_t mode)`

Create or truncate a file. Equivalent to:

```c
open(path, O_WRONLY | O_CREAT | O_TRUNC, mode)
```

**Parameters:**

| Parameter | Description |
|-----------|-------------|
| `path`    | Path to the file. Must not be `NULL` (undefined behavior, same as `creat()`). |
| `mode`    | Permission bits for new files, modified by the process umask. Ignored if the file already exists. |

**Returns:** A non-negative file descriptor on success (opened **write-only**), or `-1` on failure with `errno` set.

**Common errno values:** `EACCES`, `EISDIR`, `EMFILE`, `ENAMETOOLONG`, `ENOENT`, `ENOSPC`, `ENOTDIR`.

> **Note:** The returned file descriptor is write-only. Calling `read()` on it will fail with `EBADF`. Passing `NULL` as `path` is undefined behavior, matching `creat()`.

## Equivalence

`create(path, mode)` is equivalent to `creat(path, mode)` and to `open(path, O_WRONLY | O_CREAT | O_TRUNC, mode)`. This equivalence is verified by the parity tests in `tests/parity.c`, which run both `create()` and `creat()` on the same scenarios and assert identical results, `errno` values, file sizes, and permission modes.

## Platform Support

| Platform | Status |
|----------|--------|
| Linux    | ✅ Supported (primary target) |
| macOS    | ⚠️ Best-effort |
| Windows  | ❌ Not supported |

## Versioning and ABI Policy

libcreate uses [semantic versioning](https://semver.org/). The current version is **1.2.0**.

- The shared library SONAME is `libcreate.so.MAJOR`. It only changes on a major version bump.
- The ABI is a single function (`create`), so it is exceptionally stable.
- Version macros `LIBCREATE_VERSION_MAJOR`, `LIBCREATE_VERSION_MINOR`, `LIBCREATE_VERSION_PATCH`, and `LIBCREATE_VERSION` are available in `<create.h>`.

## Development

```sh
# Build everything
make

# Run tests (static + shared)
make test

# Run with AddressSanitizer + UndefinedBehaviorSanitizer
make sanitize

# Verify only 'create' is exported from the shared library
make check-symbols

# Build the example
make examples

# See all targets
make help
```

### CMake

```sh
# Configure, build, and test
cmake --preset default
cmake --build --preset default
ctest --preset default

# Generate coverage data
cmake --preset coverage
cmake --build --preset coverage
ctest --preset coverage
```

## License

[MIT](LICENSE)

---

*"What's one of the things I'd do differently if I were redesigning the UNIX system? I'd spell creat with an 'e'."* — Ken Thompson

You're welcome, Ken.
