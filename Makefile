# ============================================================================
# libcreate — Makefile
#
# A tiny C library that provides create(), the function Ken Thompson
# said creat() should have been named.
# ============================================================================

# ----------------------------------------------------------------------------
# Version (single source of truth)
# ----------------------------------------------------------------------------
VERSION     := $(shell cat VERSION)
VERSION_MAJOR := $(word 1,$(subst ., ,$(VERSION)))
VERSION_MINOR := $(word 2,$(subst ., ,$(VERSION)))
VERSION_PATCH := $(word 3,$(subst ., ,$(VERSION)))
SOVERSION   := $(VERSION_MAJOR)

# ----------------------------------------------------------------------------
# Install directories (GNU conventions, all overridable)
# ----------------------------------------------------------------------------
PREFIX       ?= /usr/local
EXEC_PREFIX  ?= $(PREFIX)
LIBDIR       ?= $(EXEC_PREFIX)/lib
INCLUDEDIR   ?= $(PREFIX)/include
MANDIR       ?= $(PREFIX)/share/man
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig
DOCDIR       ?= $(PREFIX)/share/doc/libcreate
DESTDIR      ?=

# ----------------------------------------------------------------------------
# Tools (respect user/environment values)
# ----------------------------------------------------------------------------
CC  ?= cc
AR  ?= ar

# ----------------------------------------------------------------------------
# Detect OS
# ----------------------------------------------------------------------------
UNAME_S := $(shell uname -s)

# ----------------------------------------------------------------------------
# Flags
#
# The project's required flags are appended so the user's CFLAGS,
# CPPFLAGS, and LDFLAGS are never replaced.
# ----------------------------------------------------------------------------
PROJ_CFLAGS   := -std=c99 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -O2
PROJ_CPPFLAGS := -D_POSIX_C_SOURCE=200809L -D_FILE_OFFSET_BITS=64 -Iinclude

# macOS hides BSD extensions such as mkdtemp() behind _DARWIN_C_SOURCE
ifeq ($(UNAME_S),Darwin)
  PROJ_CPPFLAGS += -D_DARWIN_C_SOURCE
endif

# Treat warnings as errors in CI (set WERROR=1)
ifdef WERROR
PROJ_CFLAGS += -Werror
endif

ALL_CFLAGS   = $(PROJ_CFLAGS) $(CFLAGS)
ALL_CPPFLAGS = $(PROJ_CPPFLAGS) $(CPPFLAGS)
ALL_LDFLAGS  = $(LDFLAGS)

# Shared-library specific flags
PIC_CFLAGS     := -fPIC
VISIBILITY     := -fvisibility=hidden

# ----------------------------------------------------------------------------
# Shared library naming
# ----------------------------------------------------------------------------
ifeq ($(UNAME_S),Darwin)
  # macOS dylib naming
  SHARED_EXT     := dylib
  SHARED_REAL    := libcreate.$(VERSION).$(SHARED_EXT)
  SHARED_SONAME  := libcreate.$(SOVERSION).$(SHARED_EXT)
  SHARED_LINK    := libcreate.$(SHARED_EXT)
  SONAME_FLAG    = -install_name $(LIBDIR)/$(SHARED_SONAME)
else
  # Linux / ELF
  SHARED_EXT     := so
  SHARED_REAL    := libcreate.$(SHARED_EXT).$(VERSION)
  SHARED_SONAME  := libcreate.$(SHARED_EXT).$(SOVERSION)
  SHARED_LINK    := libcreate.$(SHARED_EXT)
  SONAME_FLAG    = -Wl,-soname,$(SHARED_SONAME)
endif

STATIC_LIB := libcreate.a

# ----------------------------------------------------------------------------
# Build directory and objects
# ----------------------------------------------------------------------------
BUILD     := build
SRC       := src/create.c
OBJ_STATIC := $(BUILD)/create_static.o
OBJ_SHARED := $(BUILD)/create_shared.o

# ----------------------------------------------------------------------------
# Verbosity control (quiet by default, V=1 for verbose)
# ----------------------------------------------------------------------------
V ?= 0
ifeq ($(V),0)
  Q = @
  MSG_CC      = @printf '  CC      %s\n' $@
  MSG_AR      = @printf '  AR      %s\n' $@
  MSG_LD      = @printf '  LD      %s\n' $@
  MSG_GEN     = @printf '  GEN     %s\n' $@
  MSG_LN      = @printf '  LN      %s\n' $@
  MSG_INSTALL = @printf '  INSTALL %s\n'
  MSG_RM      = @printf '  RM      %s\n' $(BUILD)/
else
  Q =
  MSG_CC      =
  MSG_AR      =
  MSG_LD      =
  MSG_GEN     =
  MSG_LN      =
  MSG_INSTALL =
  MSG_RM      =
endif

# ============================================================================
# Targets
# ============================================================================
.PHONY: all static shared pc test check-symbols examples sanitize \
        install uninstall clean dist help

all: static shared pc

# ----------------------------------------------------------------------------
# Build directory
# ----------------------------------------------------------------------------
$(BUILD):
	$(Q)mkdir -p $(BUILD)

# ----------------------------------------------------------------------------
# Static library
# ----------------------------------------------------------------------------
static: $(BUILD)/$(STATIC_LIB)

$(BUILD)/$(STATIC_LIB): $(OBJ_STATIC) | $(BUILD)
	$(MSG_AR)
	$(Q)$(AR) rcs $@ $<

$(OBJ_STATIC): $(SRC) include/create.h | $(BUILD)
	$(MSG_CC)
	$(Q)$(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS) -MMD -MP -c -o $@ $<

# ----------------------------------------------------------------------------
# Shared library
# ----------------------------------------------------------------------------
shared: $(BUILD)/$(SHARED_REAL) $(BUILD)/$(SHARED_SONAME) $(BUILD)/$(SHARED_LINK)

$(BUILD)/$(SHARED_REAL): $(OBJ_SHARED) | $(BUILD)
	$(MSG_LD)
	$(Q)$(CC) -shared $(SONAME_FLAG) $(ALL_LDFLAGS) -o $@ $<

$(OBJ_SHARED): $(SRC) include/create.h | $(BUILD)
	$(MSG_CC)
	$(Q)$(CC) $(ALL_CPPFLAGS) $(ALL_CFLAGS) $(PIC_CFLAGS) $(VISIBILITY) -MMD -MP -c -o $@ $<

$(BUILD)/$(SHARED_SONAME): $(BUILD)/$(SHARED_REAL)
	$(MSG_LN)
	$(Q)cd $(BUILD) && ln -sf $(SHARED_REAL) $(SHARED_SONAME)

$(BUILD)/$(SHARED_LINK): $(BUILD)/$(SHARED_SONAME)
	$(MSG_LN)
	$(Q)cd $(BUILD) && ln -sf $(SHARED_SONAME) $(SHARED_LINK)

# ----------------------------------------------------------------------------
# pkg-config file
# ----------------------------------------------------------------------------
pc: $(BUILD)/libcreate.pc

$(BUILD)/libcreate.pc: libcreate.pc.in VERSION | $(BUILD)
	$(MSG_GEN)
	$(Q)sed \
	    -e 's|@PREFIX@|prefix=$(PREFIX)|' \
	    -e 's|@EXEC_PREFIX@|exec_prefix=$${prefix}|' \
	    -e 's|@LIBDIR@|libdir=$${exec_prefix}/lib|' \
	    -e 's|@INCLUDEDIR@|includedir=$${prefix}/include|' \
	    -e 's|@VERSION@|$(VERSION)|' \
	    libcreate.pc.in > $@

# ----------------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------------
TEST_SRCS   := tests/main.c tests/test.c tests/errors.c tests/parity.c
TEST_BIN_S  := $(BUILD)/test_static
TEST_BIN_D  := $(BUILD)/test_shared

TEST_CFLAGS  = $(ALL_CPPFLAGS) $(ALL_CFLAGS) -Itests -DVERSION_FILE=\"$(CURDIR)/VERSION\"

test: $(TEST_BIN_S) $(TEST_BIN_D)
	@printf '\n=== Tests (static) ===\n'
	$(Q)$(TEST_BIN_S)
	@printf '\n=== Tests (shared) ===\n'
	$(Q)LD_LIBRARY_PATH=$(BUILD) DYLD_LIBRARY_PATH=$(BUILD) $(TEST_BIN_D)

$(TEST_BIN_S): $(TEST_SRCS) include/create.h $(BUILD)/$(STATIC_LIB)
	$(MSG_CC)
	$(Q)$(CC) $(TEST_CFLAGS) -o $@ $(TEST_SRCS) $(BUILD)/$(STATIC_LIB)

$(TEST_BIN_D): $(TEST_SRCS) include/create.h $(BUILD)/$(SHARED_REAL)
	$(MSG_CC)
	$(Q)$(CC) $(TEST_CFLAGS) -o $@ $(TEST_SRCS) -L$(BUILD) -lcreate -Wl,-rpath,$(CURDIR)/$(BUILD)

# ----------------------------------------------------------------------------
# Sanitizer build
# ----------------------------------------------------------------------------
SANITIZE_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer -g

sanitize: $(TEST_SRCS) $(SRC) include/create.h | $(BUILD)
	@printf '=== Sanitizer build ===\n'
	$(Q)$(CC) $(TEST_CFLAGS) $(SANITIZE_FLAGS) -o $(BUILD)/test_sanitize \
	    $(TEST_SRCS) $(SRC)
	$(Q)$(BUILD)/test_sanitize

# ----------------------------------------------------------------------------
# Symbol check (Linux only)
# ----------------------------------------------------------------------------
check-symbols: $(BUILD)/$(SHARED_REAL)
	@printf '=== Checking exported symbols ===\n'
	$(Q)exported=$$(nm -D --defined-only $(BUILD)/$(SHARED_REAL) \
	    | awk '$$2 == "T" {print $$3}'); \
	if [ "$$exported" != "create" ]; then \
	    printf 'FAIL: unexpected exported symbols:\n%s\n' "$$exported"; \
	    exit 1; \
	fi
	@printf 'OK: only "create" is exported\n'

# ----------------------------------------------------------------------------
# Examples
# ----------------------------------------------------------------------------
examples: $(BUILD)/hello

$(BUILD)/hello: examples/hello.c static shared pc | $(BUILD)
	$(MSG_CC)
	$(Q)$(CC) $(ALL_CFLAGS) -Iinclude -o $@ examples/hello.c -L$(BUILD) -lcreate \
	    -Wl,-rpath,$(CURDIR)/$(BUILD)

# ----------------------------------------------------------------------------
# Install
# ----------------------------------------------------------------------------
install:
	@printf '  INSTALL libcreate $(VERSION)\n'
	$(Q)install -d $(DESTDIR)$(LIBDIR)
	$(Q)install -d $(DESTDIR)$(INCLUDEDIR)
	$(Q)install -d $(DESTDIR)$(PKGCONFIGDIR)
	$(Q)install -d $(DESTDIR)$(MANDIR)/man3
	$(Q)install -d $(DESTDIR)$(DOCDIR)
	$(Q)install -m 644 $(BUILD)/$(STATIC_LIB) $(DESTDIR)$(LIBDIR)/
	$(Q)install -m 755 $(BUILD)/$(SHARED_REAL) $(DESTDIR)$(LIBDIR)/
	$(Q)cd $(DESTDIR)$(LIBDIR) && ln -sf $(SHARED_REAL) $(SHARED_SONAME)
	$(Q)cd $(DESTDIR)$(LIBDIR) && ln -sf $(SHARED_SONAME) $(SHARED_LINK)
	$(Q)install -m 644 include/create.h $(DESTDIR)$(INCLUDEDIR)/
	$(Q)install -m 644 $(BUILD)/libcreate.pc $(DESTDIR)$(PKGCONFIGDIR)/
	$(Q)install -m 644 docs/create.3 $(DESTDIR)$(MANDIR)/man3/
	$(Q)install -m 644 LICENSE $(DESTDIR)$(DOCDIR)/
	$(Q)install -m 644 README.md $(DESTDIR)$(DOCDIR)/
ifeq ($(DESTDIR),)
	@printf '\n  NOTE: You may need to run ldconfig(8) to update the shared library cache.\n\n'
endif

# ----------------------------------------------------------------------------
# Uninstall
# ----------------------------------------------------------------------------
uninstall:
	@printf '  UNINSTALL libcreate\n'
	$(Q)rm -f $(DESTDIR)$(LIBDIR)/$(STATIC_LIB)
	$(Q)rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_REAL)
	$(Q)rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_SONAME)
	$(Q)rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_LINK)
	$(Q)rm -f $(DESTDIR)$(INCLUDEDIR)/create.h
	$(Q)rm -f $(DESTDIR)$(PKGCONFIGDIR)/libcreate.pc
	$(Q)rm -f $(DESTDIR)$(MANDIR)/man3/create.3
	$(Q)rm -f $(DESTDIR)$(DOCDIR)/LICENSE
	$(Q)rm -f $(DESTDIR)$(DOCDIR)/README.md
	$(Q)-rmdir $(DESTDIR)$(DOCDIR) 2>/dev/null || true
	$(Q)-rmdir $(DESTDIR)$(PKGCONFIGDIR) 2>/dev/null || true

# ----------------------------------------------------------------------------
# Clean
# ----------------------------------------------------------------------------
clean:
	$(MSG_RM)
	$(Q)rm -rf $(BUILD)

# ----------------------------------------------------------------------------
# Distribution tarball
# ----------------------------------------------------------------------------
DIST_NAME := libcreate-$(VERSION)

dist: clean
	$(MSG_GEN)
	$(Q)mkdir -p $(BUILD)/$(DIST_NAME)
	$(Q)cp -r include src tests docs examples \
	    Makefile libcreate.pc.in VERSION LICENSE README.md \
	    CHANGELOG.md CONTRIBUTING.md .clang-format .editorconfig \
	    $(BUILD)/$(DIST_NAME)/
	$(Q)tar -czf $(DIST_NAME).tar.gz -C $(BUILD) $(DIST_NAME)
	$(Q)rm -rf $(BUILD)/$(DIST_NAME)
	@printf '  Created %s\n' $(DIST_NAME).tar.gz

# ----------------------------------------------------------------------------
# Help
# ----------------------------------------------------------------------------
help:
	@printf 'libcreate $(VERSION) — build targets:\n\n'
	@printf '  all            Build static lib, shared lib, and pkg-config file (default)\n'
	@printf '  static         Build libcreate.a\n'
	@printf '  shared         Build libcreate.so with versioned symlinks\n'
	@printf '  pc             Generate libcreate.pc\n'
	@printf '  test           Build and run tests (static + shared)\n'
	@printf '  sanitize       Run tests with ASan + UBSan\n'
	@printf '  check-symbols  Verify only "create" is exported from the shared lib\n'
	@printf '  examples       Build the example program\n'
	@printf '  install        Install to $$(DESTDIR)$$(PREFIX) [default: /usr/local]\n'
	@printf '  uninstall      Remove installed files\n'
	@printf '  clean          Remove build directory\n'
	@printf '  dist           Create a source tarball\n'
	@printf '  help           Show this message\n'
	@printf '\n'
	@printf 'Key variables:\n'
	@printf '  PREFIX=%s\n' '$(PREFIX)'
	@printf '  LIBDIR=%s\n' '$(LIBDIR)'
	@printf '  DESTDIR=%s\n' '$(DESTDIR)'
	@printf '  CC=%s\n' '$(CC)'
	@printf '  V=0|1          Verbose build output (default: 0)\n'
	@printf '  WERROR=1       Treat warnings as errors\n'

# ----------------------------------------------------------------------------
# Dependency tracking (auto-generated .d files)
# ----------------------------------------------------------------------------
-include $(BUILD)/*.d
