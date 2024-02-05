pipeline_dbus_input = files('pipeline-dbus.xml')
model_dbus_input = files('model-dbus.xml')
resource_dbus_input = files('resource-dbus.xml')

# Generate GDbus header and code
gdbus_prog = find_program('gdbus-codegen', required: true)
gdbus_gen_pipeline_src = custom_target('gdbus-gencode',
  input: pipeline_dbus_input,
  output: ['pipeline-dbus.h', 'pipeline-dbus.c'],
  command: [gdbus_prog, '--interface-prefix', 'org.tizen',
            '--generate-c-code', 'pipeline-dbus',
            '--output-directory', meson.current_build_dir(),
            '@INPUT@'])

gdbus_gen_model_src = custom_target('gdbus-model-gencode',
  input: model_dbus_input,
  output: ['model-dbus.h', 'model-dbus.c'],
  command: [gdbus_prog, '--interface-prefix', 'org.tizen',
            '--generate-c-code', 'model-dbus',
            '--output-directory', meson.current_build_dir(),
            '@INPUT@'])

gdbus_gen_resource_src = custom_target('gdbus-resource-gencode',
  input: resource_dbus_input,
  output: ['resource-dbus.h', 'resource-dbus.c'],
  command: [gdbus_prog, '--interface-prefix', 'org.tizen',
            '--generate-c-code', 'resource-dbus',
            '--output-directory', meson.current_build_dir(),
            '@INPUT@'])

gdbus_gen_header_dep = declare_dependency(
  sources: [gdbus_gen_pipeline_src, gdbus_gen_model_src, gdbus_gen_resource_src])

# DBus Policy configuration
configure_file(input: 'mlops-agent.conf.in',
  output: 'mlops-agent.conf',
  install_dir: dbus_policy_dir,
  configuration: ml_agent_conf
)

# DBus System Service
configure_file(input: 'org.tizen.machinelearning.service.service.in',
  output: 'org.tizen.machinelearning.service.service',
  install_dir: dbus_system_service_dir,
  configuration: ml_agent_conf
)

# Systemd Service file
configure_file(input: 'mlops-agent.service.in',
  output: 'mlops-agent.service',
  install_dir: systemd_service_dir,
  configuration: ml_agent_conf
)
