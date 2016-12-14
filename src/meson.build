#
# target: libcrbtree.so
# We build both, a static and a shared library. We want our tests to get access
# to internals, so we link them statically.
#

libcrbtree_private = static_library('crbtree-private',
                                    ['c-rbtree.c'],
                                    c_args: [
                                        '-fvisibility=hidden',
                                        '-fno-common'
                                    ],
                                    pic: true)
install_headers('c-rbtree.h')
libcrbtree_symfile = join_paths(meson.current_source_dir(), 'libcrbtree.sym')
libcrbtree_shared = shared_library('crbtree',
                                   objects: libcrbtree_private.extract_all_objects(),
                                   install: true,
                                   soversion: 0,
                                   link_depends: libcrbtree_symfile,
                                   link_args: [
                                        '-Wl,--no-undefined',
                                        '-Wl,--version-script=@0@'.format(libcrbtree_symfile)
                                   ])
libcrbtree_dep = declare_dependency(include_directories: include_directories('.'),
                                    link_with: libcrbtree_private,
                                    version: meson.project_version())
mod_pkgconfig.generate(libraries: libcrbtree_shared,
                       version: meson.project_version(),
                       name: 'libcrbtree',
                       filebase: 'libcrbtree',
                       description: 'Red-Black Tree Implementation')

#
# target: test-api
# The test-api program explicitly links against the shared library, since it
# tests for symbol visibility.
#

test_api = executable('test-api', ['test-api.c'], link_with: libcrbtree_shared)
test('API Symbol Visibility', test_api)

#
# target: test-*
# All other tests are listed here. They link against the static library, so
# they can access internals for verification.
#

test_basic = executable('test-basic', ['test-basic.c'], link_with: libcrbtree_private)
test('Basic API Behavior', test_basic)

test_map = executable('test-map', ['test-map.c'], link_with: libcrbtree_private)
test('Generic Map', test_map)

test_parallel = executable('test-parallel', ['test-parallel.c'], link_with: libcrbtree_private)
test('Lockless Parallel Readers', test_parallel)

test_posix = executable('test-posix', ['test-posix.c'], link_with: libcrbtree_private)
test('Posix tsearch(3p) Comparison', test_posix)
