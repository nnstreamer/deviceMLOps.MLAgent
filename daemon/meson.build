# Machine Learning Agent
ml_agent_incs = include_directories('.', 'include')
ml_agent_lib_srcs = files('modules.c', 'gdbus-util.c', 'mlops-agent-interface.c',
  'mlops-agent-internal.c', 'mlops-agent-node.c',
  'pipeline-dbus-impl.cc', 'model-dbus-impl.cc', 'resource-dbus-impl.cc', 'service-db.cc')

ml_agent_deps = [
  gdbus_gen_header_dep,
  glib_dep,
  gio_dep,
  gst_dep,
  sqlite_dep,
  libsystemd_dep,
  json_glib_dep
]

if (get_option('enable-tizen'))
  ml_agent_deps += [
    dependency('capi-appfw-app-common'),
    dependency('dlog')
  ]
endif

serviceDBPath = get_option('service-db-path')
ml_agent_db_path_arg = '-DDB_PATH="' + serviceDBPath + '"'

serviceDBKeyPrefix = get_option('service-db-key-prefix')
ml_agent_db_key_prefix_arg = '-DDB_KEY_PREFIX="' + serviceDBKeyPrefix + '"'

ml_agent_shared_lib = shared_library ('mlops-agent',
  ml_agent_lib_srcs,
  dependencies: ml_agent_deps,
  include_directories: ml_agent_incs,
  install: true,
  install_dir: ml_agent_install_libdir,
  cpp_args: [ml_agent_db_path_arg, ml_agent_db_key_prefix_arg],
  version: ml_agent_version,
)

ml_agent_static_lib = static_library('mlops-agent',
  ml_agent_lib_srcs,
  dependencies: ml_agent_deps,
  include_directories: ml_agent_incs,
  install: true,
  install_dir: ml_agent_install_libdir,
  cpp_args: [ml_agent_db_path_arg, ml_agent_db_key_prefix_arg],
  pic: true,
)

ml_agent_lib = ml_agent_shared_lib
if get_option('default_library') == 'static'
  ml_agent_lib = ml_agent_static_lib
endif

ml_agent_dep = declare_dependency(
  dependencies: ml_agent_deps,
  include_directories: ml_agent_incs,
  link_with: ml_agent_lib
)

ml_agent_main_file = files('main.c')
ml_agent_executable = executable('mlops-agent',
  ml_agent_main_file,
  dependencies: ml_agent_dep,
  install: true,
  install_dir: ml_agent_install_bindir,
  c_args: [ml_agent_db_path_arg, ml_agent_db_key_prefix_arg],
  pie: true
)

configure_file(input: 'mlops-agent.pc.in', output: 'mlops-agent.pc',
  install_dir: join_paths(ml_agent_install_libdir, 'pkgconfig'),
  configuration: ml_agent_conf
)

subdir('include')
