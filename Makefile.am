# ------------------------------------------------------------------------------
# main

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory
AUTOMAKE_OPTIONS = color-tests parallel-tests

GCC_COLORS ?= 'ooh, shiny!'
export GCC_COLORS

SUBDIRS = .

# remove targets if the command fails
.DELETE_ON_ERROR:

# keep intermediate files
.SECONDARY:

# Keep the test-suite.log and Makefile around at all times
.PRECIOUS: $(TEST_SUITE_LOG) Makefile

CLEANFILES = $(BUILT_SOURCES)
DISTCLEANFILES =
EXTRA_DIST =
BUILT_SOURCES =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
bin_PROGRAMS =
noinst_PROGRAMS =
check_PROGRAMS =
TESTS =
default_tests =

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-I $(top_srcdir)/src \
	-I $(top_builddir)/src \
	$(OUR_CPPFLAGS)

AM_CFLAGS = $(OUR_CFLAGS)
AM_LDFLAGS = $(OUR_LDFLAGS)

# ------------------------------------------------------------------------------
# libcrbtree-private (used for our tests, which access internal data)

noinst_LTLIBRARIES += \
	libcrbtree-private.la

libcrbtree_private_la_SOURCES = \
	src/crbtree.h \
	src/crbtree-private.h \
	src/crbtree.c

libcrbtree_private_la_CFLAGS = \
	$(AM_CFLAGS)

libcrbtree_private_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-UNDEBUG

libcrbtree_private_la_LDFLAGS = \
	$(AM_LDFLAGS)

# ------------------------------------------------------------------------------
# libcrbtree

lib_LTLIBRARIES += \
	libcrbtree.la

libcrbtree_la_SOURCES = \
	$(libcrbtree_private_la_SOURCES)

libcrbtree_la_CFLAGS = \
	$(AM_CFLAGS)

libcrbtree_la_CPPFLAGS = \
	$(AM_CPPFLAGS)

libcrbtree_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	-version-info $(LIBCRBTREE_CURRENT):$(LIBCRBTREE_REVISION):$(LIBCRBTREE_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/libcrbtree.sym

# ------------------------------------------------------------------------------
# test-api

default_tests += \
	test-api

test_api_SOURCES = \
	src/test-api.c

test_api_LDADD = \
	libcrbtree.la # explicitly linked against public library

# ------------------------------------------------------------------------------
# test-basic

default_tests += \
	test-basic

test_basic_SOURCES = \
	src/test-basic.c

test_basic_LDADD = \
	libcrbtree-private.la

# ------------------------------------------------------------------------------
# test-map

default_tests += \
	test-map

test_map_SOURCES = \
	src/test-map.c

test_map_LDADD = \
	libcrbtree-private.la

# ------------------------------------------------------------------------------
# test suite

check_PROGRAMS += $(default_tests)
TESTS += $(default_tests)
