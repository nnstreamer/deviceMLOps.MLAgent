# plugin-parser for rpk packages

if not get_option('enable-tizen')
  message('-- enable-tizen is disabled, not building plugin-parser --')
  subdir_done()
endif

deps = [
  glib_dep,
  json_glib_dep,
  ml_agent_dep,
  dependency('dlog'),
  dependency('pkgmgr-info'),
]

plugin_parser_src = [
  'mlops-plugin-parser.cc',
]

mlops_plugin_parser = shared_library ('mlops-plugin-parser',
  plugin_parser_src,
  dependencies: deps,
  install: true,
  install_dir: plugin_parser_install_dir
)

# Configure info file
info_file_conf = configuration_data()
info_file_conf.set('metadata_name', 'mlops')
info_file_conf.set('plugin_parser_install_dir', plugin_parser_install_dir)
info_file_conf.set('plugin_parser_name', mlops_plugin_parser.full_path().split('/')[-1])
configure_file(input: 'mlops-plugin-parser.info.in',
               output: 'mlops-plugin-parser.info',
               install_dir: plugin_parser_info_txt_install_dir,
               configuration: info_file_conf)
