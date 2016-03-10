# ------------------------------------------------------------------------------
# autotools

SHELL = bash
ACLOCAL_AMFLAGS = -I build/m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory
AUTOMAKE_OPTIONS = color-tests parallel-tests

GCC_COLORS ?= 'ooh, shiny!'
export GCC_COLORS

# remove targets if the command fails
.DELETE_ON_ERROR:

# keep intermediate files
.SECONDARY:

# keep the test-suite.log and Makefile around at all times
.PRECIOUS: $(TEST_SUITE_LOG) Makefile

pkgincludedir=$(includedir)/org.bus1
pkgconfiglibdir=$(libdir)/pkgconfig

BUILT_SOURCES =
CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST =
pkginclude_HEADERS =
pkgconfiglib_DATA =
noinst_LIBRARIES =
check_PROGRAMS =
default_tests =
TESTS =

AM_CPPFLAGS = \
	-include $(top_builddir)/build/config.h \
	-I $(top_srcdir)/src \
	-I $(top_builddir)/src \
	$(OUR_CPPFLAGS)

AM_CFLAGS = $(OUR_CFLAGS)
AM_LDFLAGS = $(OUR_LDFLAGS)

# ------------------------------------------------------------------------------
# libcrbtree.a

noinst_LIBRARIES += \
	libcrbtree.a

pkginclude_HEADERS += \
	src/org.bus1/c-rbtree.h

libcrbtree_a_SOURCES = \
	src/libcrbtree.sym \
	src/c-rbtree-private.h \
	src/c-rbtree.c

# ------------------------------------------------------------------------------
# libcrbtree.so.0

libcrbtree.so.0: libcrbtree.a $(top_srcdir)/src/libcrbtree.sym
	$(AM_V_CCLD)$(CCLD) $(AM_CFLAGS) $(AM_LDFLAGS) \
	-shared \
	-Wl,-soname=$@ \
	-Wl,--version-script=$(top_srcdir)/src/libcrbtree.sym \
	-Wl,--whole-archive libcrbtree.a -Wl,--no-whole-archive \
	-o $@

CLEANFILES += \
	libcrbtree.so.0

all-local: libcrbtree.so.0

install-exec-hook:
	$(MKDIR_P) $(DESTDIR)$(libdir)
	cp libcrbtree.so.0 $(DESTDIR)$(libdir)
	$(AM_V_LN)ln -sf libcrbtree.so.0 $(DESTDIR)$(libdir)/libcrbtree.so

uninstall-hook:
	rm -f $(DESTDIR)$(libdir)/libcrbtree.so.0

%.pc: %.pc.in
	$(AM_V_GEN)$(SED) \
		-e 's,@includedir\@,$(includedir),g' \
		-e 's,@libdir\@,$(libdir),g' \
		-e 's,@VERSION\@,$(VERSION),g' \
		$< > $@

pkgconfiglib_DATA += \
	src/libcrbtree.pc

EXTRA_DIST += \
	src/libcrbtree.pc.in

CLEANFILES += \
	src/libcrbtree.pc
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
# test suite

AM_TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(abs_builddir)
check_PROGRAMS += $(default_tests)
TESTS += $(default_tests)

# ------------------------------------------------------------------------------¶
# check "make install" directory tree

install-tree: all
	rm -rf $(abs_srcdir)/install-tree
	$(MAKE) install DESTDIR=$(abs_srcdir)/install-tree
	tree $(abs_srcdir)/install-tree
.PHONY: install-tree
