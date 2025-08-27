project('ml-agent', 'c', 'cpp',
  version: '1.8.8',
  license: ['Apache-2.0'],
  meson_version: '>=0.50.0',
  default_options: [
    'b_asneeded=false',
    'werror=true',
    'warning_level=1',
    'c_std=gnu89',
    'cpp_std=c++14'
  ]
)

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

# Dependencies
glib_dep = dependency('glib-2.0')
gobject_dep = dependency('gobject-2.0')
gmodule_dep = dependency('gmodule-2.0')
if host_machine.system() == 'windows'
  gio_dep = dependency('gio-2.0')
else
  gio_dep = [dependency('gio-2.0'), dependency('gio-unix-2.0')]
endif
gst_dep = dependency('gstreamer-1.0')
libsystemd_dep = dependency('libsystemd')
sqlite_dep = dependency('sqlite3')
json_glib_dep = dependency('json-glib-1.0')

# Set version info
ml_agent_version = meson.project_version()
ml_agent_version_split = ml_agent_version.split('.')

add_project_arguments('-DVERSION="' + ml_agent_version + '"', language: ['c', 'cpp'])
add_project_arguments('-DVERSION_MAJOR=' + ml_agent_version[0], language: ['c', 'cpp'])
add_project_arguments('-DVERSION_MINOR=' + ml_agent_version[1], language: ['c', 'cpp'])
add_project_arguments('-DVERSION_MICRO=' + ml_agent_version[2], language: ['c', 'cpp'])

# Define warning flags for c and cpp
warning_flags = [
  '-Werror=address',
  '-Werror=array-bounds',
  '-Werror=empty-body',
  '-Werror=format=2',
  '-Werror=init-self',
  '-Werror=int-to-pointer-cast',
  '-Werror=main',
  '-Werror=missing-braces',
  '-Werror=nonnull',
  '-Werror=return-type',
  '-Werror=sequence-point',
  '-Werror=trigraphs',
  '-Werror=write-strings',
  '-Wredundant-decls',
  '-Wmissing-braces',
  '-Wmaybe-uninitialized',
  '-Wwrite-strings',
  '-Wformat',
  '-Wformat-nonliteral',
  '-Wformat-security',
  '-Winit-self',
  '-Waddress',
  '-Wno-multichar',
  '-Wvla',
  '-Wpointer-arith'
]

warning_c_flags = [
  '-Werror=implicit',
  '-Werror=pointer-to-int-cast',
  '-Werror=undef',
  '-Wmissing-declarations',
  '-Wmissing-include-dirs',
  '-Wmissing-prototypes',
  '-Wnested-externs',
  '-Waggregate-return',
  '-Wold-style-definition',
  '-Wdeclaration-after-statement'
]

if glib_dep.version() < '2.67.1'
  warning_c_flags += '-Wno-incompatible-pointer-types'
endif

# Setup warning flags for c and cpp
add_project_arguments(cc.get_supported_arguments(warning_c_flags), language: 'c')
add_project_arguments(cc.get_supported_arguments(warning_flags), language: 'c')
add_project_arguments(cxx.get_supported_arguments(warning_flags), language: 'cpp')

# Set project args
if get_option('enable-tizen')
  # Pass __TIZEN__ to the compiler
  add_project_arguments('-D__TIZEN__=1', language: ['c', 'cpp'])
endif

# Set install path
ml_agent_install_prefix = get_option('prefix')
ml_agent_install_libdir = join_paths(ml_agent_install_prefix, get_option('libdir'))
ml_agent_install_bindir = join_paths(ml_agent_install_prefix, get_option('bindir'))
ml_agent_install_includedir = join_paths(ml_agent_install_prefix, get_option('includedir'))
ml_agent_install_inidir = get_option('sysconfdir')

dbus_policy_dir = join_paths(get_option('sysconfdir'), 'dbus-1', 'system.d')
dbus_system_service_dir = join_paths(ml_agent_install_prefix, 'share', 'dbus-1', 'system-services')
systemd_service_dir = join_paths(ml_agent_install_prefix, 'lib', 'systemd', 'system')

plugin_parser_install_dir = join_paths(get_option('sysconfdir'), 'package-manager', 'parserlib', 'metadata')
plugin_parser_info_txt_install_dir = join_paths(ml_agent_install_prefix, 'share', 'parser-plugins')

# Set default configuration
ml_agent_conf = configuration_data()
ml_agent_conf.set('VERSION', ml_agent_version)
ml_agent_conf.set('PREFIX', ml_agent_install_prefix)
ml_agent_conf.set('EXEC_PREFIX', ml_agent_install_bindir)
ml_agent_conf.set('LIB_INSTALL_DIR', ml_agent_install_libdir)
ml_agent_conf.set('INCLUDE_INSTALL_DIR', ml_agent_install_includedir)

subdir('dbus')
subdir('daemon')
subdir('plugin-parser')
subdir('tests')
