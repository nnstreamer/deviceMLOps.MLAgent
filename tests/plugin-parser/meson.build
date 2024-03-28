# test plugin-parser
if not get_option('enable-tizen')
  message('-- enable-tizen is disabled, not building plugin-parser unittest --')
  subdir_done()
endif

gmock_dep = dependency('gmock', required: false)
if not gmock_dep.found()
  message('-- gmock is not found, not building  plugin-parser unittest --')
  subdir_done()
endif

unittest_plugin_parser = executable('unittest_mlops_plugin_parser',
  'unittest_mlops_plugin_parser.cc',
  dependencies: [
    gmodule_dep,
    gtest_dep,
    gmock_dep,
    ml_agent_test_dep,
  ],
  install: get_option('install-test'),
  install_dir: unittest_install_dir
)
