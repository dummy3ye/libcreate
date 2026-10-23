# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0] - 2026-10-23

### Added

- CMake build system with static/shared libraries, CTest integration, install
  rules, and CMake presets.
- Coverage preset and Codecov reporting in CI.

## [1.0.1] - 2026-10-23

### Fixed

- Compile the test suite on macOS by defining `_DARWIN_C_SOURCE`, which exposes the BSD `mkdtemp(3)` declaration.

## [1.0.0] - 2026-10-23

### Added

- `create()` function: equivalent to POSIX `creat()`, implemented via `open()`.
- Public header `create.h` with version macros and visibility attributes.
- Static library (`libcreate.a`) and shared library (`libcreate.so`) builds.
- pkg-config file (`libcreate.pc`).
- Man page (`create.3`).
- Comprehensive test suite: basic functionality, error paths, and parity with `creat()`.
- AddressSanitizer and UndefinedBehaviorSanitizer support (`make sanitize`).
- Exported symbol verification (`make check-symbols`).
- Version consistency checks between `VERSION` file and header macros.
- Example program with pkg-config integration.
- CI workflow for Ubuntu (GCC + Clang).
- Tag-driven release workflow that verifies the version, builds and tests, and publishes a source tarball with checksums.
- `install`, `uninstall`, and `dist` targets with full `DESTDIR` support.
