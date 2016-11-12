# ------------------------------------------------------------------------------
# autotools

SHELL = bash
ACLOCAL_AMFLAGS = -I build/m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory
AUTOMAKE_OPTIONS = color-tests parallel-tests

# remove targets if the command fails
.DELETE_ON_ERROR:

# keep intermediate files
.SECONDARY:

# keep the test-suite.log and Makefile around at all times
.PRECIOUS: $(TEST_SUITE_LOG) Makefile

pkgconfiglibdir=$(libdir)/pkgconfig

BUILT_SOURCES =
CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST =
include_HEADERS =
pkgconfiglib_DATA =
noinst_LIBRARIES =
check_PROGRAMS =
default_tests =
TESTS =

AM_CPPFLAGS = \
	-include $(top_builddir)/build/config.h \
	-I $(top_srcdir)/src \
	-I $(top_builddir)/src \
	-Wp,-D_FORTIFY_SOURCE=2 \
	-Wno-typedef-redefinition \
	-Wno-gnu-variable-sized-type-not-at-end

AM_CFLAGS = \
	-g \
	-O2 \
	-pipe \
	-ftrapv \
	-ffast-math \
	-fno-common \
	-fdiagnostics-show-option \
	-fno-strict-aliasing \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections \
	-fstack-protector \
	-fstack-protector-strong \
	-fPIC \
	--param=ssp-buffer-size=4 \
	-Wall \
	-Wextra \
	-Wno-inline \
	-Wundef \
	-Wformat=2 \
	-Wformat-security \
	-Wformat-nonliteral \
	-Wsign-compare \
	-Wmissing-include-dirs \
	-Wold-style-definition \
	-Wpointer-arith \
	-Winit-self \
	-Wdeclaration-after-statement \
	-Wfloat-equal \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wredundant-decls \
	-Wmissing-declarations \
	-Wmissing-noreturn \
	-Wshadow \
	-Wendif-labels \
	-Wstrict-aliasing=2 \
	-Wwrite-strings \
	-Wno-long-long \
	-Wno-overlength-strings \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers \
	-Wno-unused-result \
	-Werror=overflow \
	-Wdate-time \
	-Wnested-externs

AM_LDFLAGS = \
	-Wl,--as-needed \
	-Wl,--no-undefined \
	-Wl,--gc-sections \
	-Wl,-z,relro \
	-Wl,-z,now \
	-pie

# ------------------------------------------------------------------------------
# library archive

noinst_LIBRARIES += \
	libcrbtree.a

libcrbtree_a_SOURCES = \
	src/libcrbtree.sym \
	src/c-rbtree-private.h \
	src/c-rbtree.c \
	src/c-rbtree.h

# ------------------------------------------------------------------------------
# shared library built from archive

all-local: libcrbtree.so.0

include_HEADERS += \
	src/c-rbtree.h

libcrbtree.so.0: libcrbtree.a $(top_srcdir)/src/libcrbtree.sym
	$(AM_V_CCLD)$(LINK) -shared \
	-Wl,-soname=$@ \
	-Wl,--version-script=$(top_srcdir)/src/libcrbtree.sym \
	-Wl,--whole-archive libcrbtree.a -Wl,--no-whole-archive

CLEANFILES += \
	libcrbtree.so.0

install-exec-local:
	@echo " $(MKDIR_P) '$(DESTDIR)$(libdir)'"; \
		$(MKDIR_P) "$(DESTDIR)$(libdir)" || exit 1
	@echo " $(INSTALL) libcrbtree.so.0 '$(DESTDIR)$(libdir)'"; \
		$(INSTALL) libcrbtree.so.0 "$(DESTDIR)$(libdir)" || exit $$?
	@echo " $(LN_S) -f libcrbtree.so.0 '$(DESTDIR)$(libdir)/libcrbtree.so'"; \
		$(LN_S) -f libcrbtree.so.0 "$(DESTDIR)$(libdir)/libcrbtree.so" || exit $$?

uninstall-local:
	@test ! -d "$(DESTDIR)$(libdir)" || \
		{ echo " ( cd '$(DESTDIR)$(libdir)' && rm -f libcrbtree.so.0 )"; \
		  rm -f "$(DESTDIR)$(libdir)/libcrbtree.so.0"; }

%.pc: %.pc.in
	$(AM_V_GEN)$(SED) \
		-e 's,@includedir\@,$(includedir),g' \
		-e 's,@libdir\@,$(libdir),g' \
		-e 's,@VERSION\@,$(VERSION),g' \
		$< > $@

pkgconfiglib_DATA += \
	src/c-rbtree.pc

EXTRA_DIST += \
	src/c-rbtree.pc.in

CLEANFILES += \
	src/c-rbtree.pc

# ------------------------------------------------------------------------------
# test-api

default_tests += \
	test-api

test_api_SOURCES = \
	src/test-api.c

test_api_LDADD = \
	libcrbtree.so.0 # explicitly linked against public library

# ------------------------------------------------------------------------------
# test-basic

default_tests += \
	test-basic

test_basic_SOURCES = \
	src/test-basic.c

test_basic_LDADD = \
	libcrbtree.a

# ------------------------------------------------------------------------------
# test-map

default_tests += \
	test-map

test_map_SOURCES = \
	src/test-map.c

test_map_LDADD = \
	libcrbtree.a

# ------------------------------------------------------------------------------
# test-parallel

default_tests += \
	test-parallel

test_parallel_SOURCES = \
	src/test-parallel.c

test_parallel_LDADD = \
	libcrbtree.a

# ------------------------------------------------------------------------------
# test-posix

default_tests += \
	test-posix

test_posix_SOURCES = \
	src/test-posix.c

test_posix_LDADD = \
	libcrbtree.a

# ------------------------------------------------------------------------------
# test suite

AM_TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(abs_builddir)
check_PROGRAMS += $(default_tests)
TESTS += $(default_tests)

# ------------------------------------------------------------------------------¶
# check "make install" directory tree

install-tree: all
	rm -rf $(abs_builddir)/install-tree
	$(MAKE) install DESTDIR=$(abs_builddir)/install-tree
	tree $(abs_builddir)/install-tree
.PHONY: install-tree
