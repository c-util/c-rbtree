project(
        'c-rbtree',
        'c',
        version: '3',
        license: 'Apache',
        default_options: [
                'c_std=c11'
        ],
)
project_description = 'Intrusive Red-Black Tree Collection'

cc = meson.get_compiler('c')
if cc.has_argument('-Wno-unused-parameter')
    add_project_arguments('-Wno-unused-parameter', language: 'c')
endif

add_project_arguments('-D_GNU_SOURCE', language: 'c')
mod_pkgconfig = import('pkgconfig')

sub_cstdaux = subproject('c-stdaux')

dep_cstdaux = sub_cstdaux.get_variable('libcstdaux_dep')

subdir('src')
