# Contributing to libcreate

Thanks for your interest! libcreate is deliberately tiny, and its scope is intentionally minimal.

## Building

```sh
make            # Build static and shared libraries
make test       # Run the test suite
make sanitize   # Run tests under ASan + UBSan
```

## Code Style

- Follow the `.clang-format` configuration (K&R-ish, 4-space indent, ~100 column limit).
- Use the `.editorconfig` settings.
- Keep comments minimal and purposeful in source files; document the public header thoroughly.

## Running Tests

Tests require no external framework. Run them with:

```sh
make test
```

Tests create and clean up a temporary directory. They work both as root and as a regular user (the `EACCES` test is skipped when running as root).

## Scope Policy

The scope of this library is **deliberately tiny**. The only function is `create()`, which must behave identically to `creat()`. Proposals for additional features (convenience wrappers, new flags, etc.) are out of scope for the current version.

If you believe you've found a case where `create()` does not match `creat()`, that's a bug — please report it!

## Submitting Changes

1. Fork the repository and create a feature branch.
2. Make your changes in small, focused commits.
3. Ensure `make test`, `make sanitize`, and `make check-symbols` all pass.
4. Open a pull request with a clear description of what you changed and why.

## Releasing

1. Update `VERSION` and the version macros in `include/create.h` to the new
   version. The test suite verifies they match.
2. Add a matching `## [x.y.z] - YYYY-MM-DD` section to `CHANGELOG.md`.
3. Commit the changes, then tag the release and push the tag:

   ```sh
   git tag -a v1.0.0 -m "libcreate 1.0.0"
   git push origin v1.0.0
   ```

The `Release` workflow runs on `v*` tags. It verifies that the tag matches
`VERSION`, builds and tests the project, creates a source tarball with
`make dist`, writes `SHA256SUMS`, and publishes a GitHub release using the
matching `CHANGELOG.md` section as the release notes.
