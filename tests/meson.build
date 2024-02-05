# Install path for unittest
test_base_dir = join_paths(ml_agent_install_bindir, 'ml-test')
unittest_install_dir = join_paths(test_base_dir, 'unittests')

ml_agent_lib_objs = ml_agent_lib.extract_objects(ml_agent_lib_srcs)
ml_agent_test_both_lib = both_libraries('mlops-agent-test',
  dependencies: ml_agent_deps,
  include_directories: ml_agent_incs,
  install: true,
  install_dir: ml_agent_install_libdir,
  cpp_args: ['-DDB_PATH="."', ml_agent_db_key_prefix_arg],
  objects: ml_agent_lib_objs,
  version: ml_agent_version,
  pic: true
)

ml_agent_test_lib = ml_agent_test_both_lib.get_shared_lib()
if get_option('default_library') == 'static'
  ml_agent_test_lib = ml_agent_test_both_lib.get_static_lib()
endif

ml_agent_test_dep = declare_dependency(
  dependencies: ml_agent_deps,
  include_directories: ml_agent_incs,
  link_with: ml_agent_test_lib
)

ml_agent_main_objs = ml_agent_executable.extract_objects(ml_agent_main_file)
executable('mlops-agent-test',
  dependencies: ml_agent_test_dep,
  install: true,
  install_dir: test_base_dir,
  c_args: ['-DDB_PATH="."', ml_agent_db_key_prefix_arg],
  objects: ml_agent_main_objs
)

subdir('services')
# Run tests
if get_option('enable-test')
  gtest_dep = dependency('gtest', required: false)
  if gtest_dep.found()
    subdir('daemon')
  else
    warning('The enable-test option requires google-test.')
  endif
endif
