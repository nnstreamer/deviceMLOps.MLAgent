# Set dependency and test-env
testenv = environment()
testenv.set('MLAGENT_SOURCE_ROOT_PATH', meson.source_root())
testenv.set('MLAGENT_BUILD_ROOT_PATH', meson.build_root())

unittest_ml_agent = executable('unittest_ml_agent',
  'unittest_ml_agent.cc',
  dependencies: [gtest_dep, ml_agent_test_dep],
  install: get_option('install-test'),
  install_dir: unittest_install_dir
)
test('unittest_ml_agent', unittest_ml_agent, env: testenv, timeout: 100)

unittest_service_db = executable('unittest_service_db',
  'unittest_service_db.cc',
  dependencies: [gtest_dep, ml_agent_test_dep],
  install: get_option('install-test'),
  install_dir: unittest_install_dir
)
test('unittest_service_db', unittest_service_db, env: testenv, timeout: 100)

unittest_gdbus_util = executable('unittest_gdbus_util',
  'unittest_gdbus_util.cc',
  dependencies: [gtest_dep, ml_agent_test_dep],
  link_with: ml_agent_lib,
  install: get_option('install-test'),
  install_dir: unittest_install_dir
)
test('unittest_gdbus_util', unittest_gdbus_util, env: testenv, timeout: 100)
