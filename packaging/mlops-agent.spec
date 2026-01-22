###########################################################################
# Default features for Tizen release
# If you want to build RPM for other Linux distro, you may need to
# touch these values for your needs.

# Create an option to build without tizen (`--without tizen`), thus default
# building with it
# https://rpm-software-management.github.io/rpm/manual/conditionalbuilds.html
%bcond_without tizen

###########################################################################
# Macros for building and testing option control
# Note that, when changing install path for unittest,
# you should update 'run-unittest.sh' in packaging directory.
%define		builddir build
%define		source_root %{_builddir}/%{?buildsubdir}
%define		test_script %{source_root}/packaging/run_unittests.sh
%define		test_base_dir %{_bindir}/ml-test

# Note that debug packages generate an additional build and storage cost.
# If you do not need debug packages, run '$ gbs build ... --define "_skip_debug_rpm 1"'.
%if "%{?_skip_debug_rpm}" == "1"
%global debug_package %{nil}
%global __debug_install_post %{nil}
%endif

###########################################################################
# Define build options

# Below features are used for unittest.
# Do not add neural network dependency to ML-Agent.
%define enable_test -Denable-test=false
%define install_test -Dinstall-test=false

# To generate gcov package, --define "gcov 1"
%if 0%{?gcov:1}
%define		unit_test 1
%define		release_test 1
%define		testcoverage 1
%endif

# To set prefix, use this line
### define service_db_key_prefix -Dservice-db-key-prefix='some-prefix'
%define service_db_key_prefix %{nil}

###########################################################################
# Package / sub-package definitions
Name:		mlops-agent
Summary:	AI service agent for MLOps and ML(Machine Learning) API
# Synchronize the version information for ML-Agent.
# 1. Tizen   : ./packaging/mlops-agent.spec
# 2. Meson   : ./meson.build
Version:	1.8.8
Release:	0
Group:		Machine Learning/ML Framework
Packager:	Sangjung Woo <sangjung.woo@samsung.com>
License:	Apache-2.0
Source0:	mlops-agent-%{version}.tar
Source1001:	mlops-agent.manifest

## Define runtime requirements ##
Requires:	libmlops-agent = %{version}-%{release}
Requires:	dbus-1

## Define build requirements ##
BuildRequires:	meson >= 0.50.0
BuildRequires:	pkgconfig(glib-2.0)
BuildRequires:	pkgconfig(gstreamer-1.0)
BuildRequires:	pkgconfig(libsystemd)
BuildRequires:	pkgconfig(sqlite3)
BuildRequires:	pkgconfig(json-glib-1.0)

%if %{with tizen}
BuildRequires:	pkgconfig(dlog)
BuildRequires:	pkgconfig(libtzplatform-config)
BuildRequires:	pkgconfig(capi-appfw-app-common)
BuildRequires:	pkgconfig(pkgmgr-info)
%endif # tizen

# For test
%if 0%{?unit_test}
# g_test_dbus_up requires dbus
BuildRequires:	dbus-1
BuildRequires:	pkgconfig(gtest)

%if 0%{?testcoverage}
# to be compatible with gcc-9, lcov should have a higher version than 1.14.1
BuildRequires:	lcov
%endif
%endif # unit_test

%description
AI service agent for ML(Machine Learning) API and NNStreamer.

%package -n libmlops-agent
Summary:	Library that exports interfaces provided by Machine Learning Agent
Group:		Machine Learning/ML Framework = %{version}-%{release}
%description -n libmlops-agent
Shared library to export interfaces provided by the Machine Learning Agent.

%package -n libmlops-agent-devel
Summary:	Development headers and static library for interfaces provided by Machine Learning Agent
Group:		Machine Learning/ML Framework
Requires:	libmlops-agent = %{version}-%{release}
%description -n libmlops-agent-devel
Development headers and static library for interfaces provided by Machine Learning Agent.

%package test
Summary:	ML service agent for testing purposes
Requires:	dbus
%description test
ML agent binary for the testing purposes
This package provides the ML agent (daemon) to the other packages that
require testing with the ML Agent service.

%if 0%{?unit_test}
%if 0%{?release_test}
%package unittests
Summary:	Unittests for Machine Learning Agent
Group:		Machine Learning/ML Framework
Requires:	mlops-agent-test = %{version}-%{release}
%description unittests
Unittests for Machine Learning Agent.
%endif

%if 0%{?gcov:1}
%package gcov
Summary:	Machine Learning Agent gcov objects
Group:		Machine Learning/ML Framework
%description gcov
Machine Learning Agent gcov objects.
%endif

%if 0%{?testcoverage}
%package unittest-coverage
Summary:	Unittest coverage result for Machine Learning Agent
%description unittest-coverage
HTML pages of lcov results of Machine Learning Agent generated during rpm build.
%endif
%endif # unit_test

%prep
%setup -q
cp %{SOURCE1001} .

%build
# Remove compiler flags for meson to decide the cpp version
CXXFLAGS=`echo $CXXFLAGS | sed -e "s|-std=gnu++11||"`

%if 0%{?unit_test}
%define enable_test -Denable-test=true

%if 0%{?release_test}
%define install_test -Dinstall-test=true
%endif # release_test

%if 0%{?testcoverage}
# To test coverage, disable optimizations (and should unset _FORTIFY_SOURCE to use -O0)
CFLAGS=`echo $CFLAGS | sed -e "s|-O[1-9]|-O0|g"`
CFLAGS=`echo $CFLAGS | sed -e "s|-Wp,-D_FORTIFY_SOURCE=[1-9]||g"`
CXXFLAGS=`echo $CXXFLAGS | sed -e "s|-O[1-9]|-O0|g"`
CXXFLAGS=`echo $CXXFLAGS | sed -e "s|-Wp,-D_FORTIFY_SOURCE=[1-9]||g"`
# also, use the meson's base option, -Db_coverage, instead of --coverage/-fprofile-arcs and -ftest-coverage
%endif # testcoverage
%endif # unit_test

rm -rf %{builddir}
meson setup --buildtype=plain --prefix=%{_prefix} --sysconfdir=%{_sysconfdir} --libdir=%{_libdir} \
	--bindir=%{_bindir} --includedir=%{_includedir} \
	%{enable_test} %{install_test} %{?testcoverage:-Db_coverage=true} \
	%{?with_tizen:-Denable-tizen=true} \
	-Dservice-db-path=%{?with_tizen:%{TZ_SYS_GLOBALUSER_DB}}%{!?with_tizen:%{expand:%{?service_db_path}}%{?!service_db_path:.}} \
	%{service_db_key_prefix} \
	%{builddir}

meson compile -C %{builddir} %{?_smp_mflags}

export MLAGENT_SOURCE_ROOT_PATH=$(pwd)
export MLAGENT_BUILD_ROOT_PATH=$(pwd)/%{builddir}

%if 0%{?testcoverage}
# Capture initial zero coverage data. This will be merged with actual coverage data later.
# This is to prevent null gcda file error if the test is not performed (in case of gcov package generation mode).
pushd %{builddir}
lcov -i -c -o unittest_base.info -d . -b $(pwd) --ignore-errors mismatch --exclude "unittest*"
popd
%endif # testcoverage

# If gcov package generation is enabled, pass the test from GBS.
%if 0%{?unit_test} && !0%{?gcov}
bash %{test_script} ./tests/daemon/unittest_ml_agent
bash %{test_script} ./tests/daemon/unittest_service_db
bash %{test_script} ./tests/daemon/unittest_gdbus_util
bash %{test_script} ./tests/plugin-parser/unittest_mlops_plugin_parser
%endif # unit_test

%install
meson install -C %{builddir} --destdir=%{buildroot}

%if 0%{?unit_test}
%if 0%{?testcoverage}
# 'lcov' generates the date format with UTC time zone by default. Let's replace UTC with KST.
# If you can get a root privilege, run ln -sf /usr/share/zoneinfo/Asia/Seoul /etc/localtime
TZ='Asia/Seoul'; export TZ

# Get commit info
VCS=`cat ${RPM_SOURCE_DIR}/mlops-agent.spec | grep "^VCS:" | sed "s|VCS:\\W*\\(.*\\)|\\1|"`

# Create human readable coverage report web page.
# Generate report and exclude files which are generated by gdbus-codegen and external files in /usr/*.
# TODO: the --no-external option is removed to include machine-learning-agent related source files.
# Restore this option when there is proper way to include those source files.
pushd %{builddir}
# Set different lcov options for Tizen/lcov versions.
%if 0%{tizen_version_major} >= 9
lcov -t 'ML-Agent unittest coverage' -o unittest_test.info -c -d . -b $(pwd) --ignore-errors mismatch,empty --exclude "unittest*"
lcov -a unittest_base.info -a unittest_test.info -o unittest_total.info --ignore-errors empty
lcov -r unittest_total.info "*/tests/*" "*/meson*/*" "*/*@sha/*" "*/*.so.p/*" "*/*tizen*" "*/*-dbus.c" "/usr/*" -o unittest-filtered.info --ignore-errors graph,unused
%else
lcov -t 'ML-Agent unittest coverage' -o unittest.info -c -d . -b $(pwd)
lcov -r unittest.info "*/tests/*" "*/meson*/*" "*/*@sha/*" "*/*.so.p/*" "*/*tizen*" "*/*-dbus.c" "/usr/*" -o unittest-filtered.info
%endif # tizen_version_major >= 9
# Visualize the report
genhtml -o result unittest-filtered.info -t "ML-Agent %{version}-%{release} ${VCS}" --ignore-errors source -p ${RPM_BUILD_DIR}

mkdir -p %{buildroot}%{_datadir}/ml-agent/unittest/
cp -r result %{buildroot}%{_datadir}/ml-agent/unittest/
popd

%if 0%{?gcov:1}
builddir=$(basename $PWD)
gcno_obj_dir=%{buildroot}%{_datadir}/gcov/obj/%{name}/"$builddir"
mkdir -p "$gcno_obj_dir"
find . -name '*.gcno' ! -path "*/tests/*" ! -name "meson-generated*" ! -name "sanitycheck*" ! -path "*tizen*" -exec cp --parents '{}' "$gcno_obj_dir" ';'

mkdir -p %{buildroot}%{_bindir}/tizen-unittests/%{name}
install -m 0755 packaging/run-unittest.sh %{buildroot}%{_bindir}/tizen-unittests/%{name}
%endif

%endif # test coverage
%endif # unit_test

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%manifest mlops-agent.manifest
%license LICENSE
%attr(0755,root,root) %{_bindir}/mlops-agent
%attr(0644,root,root) %{_unitdir}/mlops-agent.service
%attr(0644,root,root) %config %{_sysconfdir}/dbus-1/system.d/mlops-agent.conf
%attr(0644,root,root) %{_datadir}/dbus-1/system-services/org.tizen.machinelearning.service.service
%{_sysconfdir}/package-manager/parserlib/metadata/libmlops-plugin-parser.so
%{_datadir}/parser-plugins/mlops-plugin-parser.info

%files -n libmlops-agent
%manifest mlops-agent.manifest
%license LICENSE
%{_libdir}/libmlops-agent.so.*

%files -n libmlops-agent-devel
%manifest mlops-agent.manifest
%{_libdir}/libmlops-agent.so
%{_libdir}/libmlops-agent.a
%{_includedir}/ml/mlops-agent-interface.h
%{_libdir}/pkgconfig/mlops-agent.pc

%files test
%manifest mlops-agent.manifest
%license LICENSE
%attr(0755,root,root) %{test_base_dir}/mlops-agent-test
%attr(0755,root,root) %{test_base_dir}/services/org.tizen.machinelearning.service.service
%{_libdir}/libmlops-agent-test.*

%if 0%{?unit_test}
%if 0%{?release_test}
%files unittests
%manifest mlops-agent.manifest
%{test_base_dir}/unittests
%if 0%{?gcov:1}
%{_bindir}/tizen-unittests/%{name}/run-unittest.sh
%endif # gcov
%endif # release_test

%if 0%{?gcov:1}
%files gcov
%{_datadir}/gcov/obj/*
%endif # gcov

%if 0%{?testcoverage}
%files unittest-coverage
%{_datadir}/ml-agent/unittest/*
%endif # testcoverage
%endif # unit_test

%changelog
* Wed Aug 27 2025 Sangjung Woo <sangjung.woo@samsung.com>
- Release of 1.8.8 (Tizen 10.0 M2)

* Mon Sep 02 2024 Sangjung Woo <sangjung.woo@samsung.com>
- Release of 1.8.6 (Tizen 9.0 M2)

* Thu Feb 08 2024 Sangjung Woo <sangjung.woo@samsung.com>
- Rename the package prefix to mlops-agent

* Mon Dec 11 2023 Sangjung Woo <sangjung.woo@samsung.com>
- Started ML-Agent packaging for 1.8.5 (Initial version sync to ML API)
