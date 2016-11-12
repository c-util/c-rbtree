# ------------------------------------------------------------------------------
# autotools

SHELL = bash
ACLOCAL_AMFLAGS = -I build/m4 ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS = color-tests parallel-tests
AM_MAKEFLAGS = --no-print-directory
AM_TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(abs_builddir)
check_PROGRAMS = $(default_tests)
TESTS = $(default_tests)
CLEANFILES =
EXTRA_DIST =

# remove targets if the command fails
.DELETE_ON_ERROR:

# keep intermediate files
.SECONDARY:

# keep the test-suite.log and Makefile around at all times
.PRECIOUS: $(TEST_SUITE_LOG) Makefile

AM_CPPFLAGS = \
	-include $(top_builddir)/build/config.h \
	-I $(top_srcdir)/src \
	-I $(top_builddir)/src

AM_CFLAGS = \
	-fdata-sections \
	-ffunction-sections \
	-fvisibility=hidden \
	-fno-common \
	-fPIC \
	-g \
	-O2 \
	-Wall \
	-Wundef \
	-Wextra \
	-Wshadow \
	-Winit-self \
	-Wformat=2 \
	-Wformat-security \
	-Wformat-nonliteral \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wdeclaration-after-statement \
	-Wredundant-decls \
	-Wno-unused-parameter \
	-Wno-inline

AM_LDFLAGS = \
	-Wl,--as-needed \
	-Wl,--no-undefined \
	-Wl,--gc-sections \
	-Wl,-z,relro \
	-Wl,-z,now \
	-pie

# ------------------------------------------------------------------------------
# libcrbtree.a

libcrbtree_a_SOURCES = \
	src/c-rbtree.h \
	src/c-rbtree-private.h \
	src/c-rbtree.c

# ------------------------------------------------------------------------------
# test-api

test_api_SOURCES = src/test-api.c
test_api_LDADD = libcrbtree.so.0 # explicitly linked against public library

# ------------------------------------------------------------------------------
# test-basic

test_basic_SOURCES = src/test-basic.c
test_basic_LDADD = libcrbtree.a

# ------------------------------------------------------------------------------
# test-map

test_map_SOURCES = src/test-map.c
test_map_LDADD = libcrbtree.a

# ------------------------------------------------------------------------------
# test-parallel

test_parallel_SOURCES = src/test-parallel.c
test_parallel_LDADD = libcrbtree.a

# ------------------------------------------------------------------------------
# test-posix

test_posix_SOURCES = src/test-posix.c
test_posix_LDADD = libcrbtree.a

# ------------------------------------------------------------------------------
# targets

include_HEADERS = src/c-rbtree.h
noinst_LIBRARIES = libcrbtree.a
SOLIBRARIES = libcrbtree.so.0
default_tests = \
	test-api \
	test-basic \
	test-map \
	test-parallel \
	test-posix

# ------------------------------------------------------------------------------
# support for $(SOLIBRARIES) (must be after its assignment)

include solibraries.mk
