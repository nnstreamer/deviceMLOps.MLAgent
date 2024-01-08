###########################################################################
# Default features for Tizen release
# If you want to build RPM for other Linux distro, you may need to
# touch these values for your needs.
%if %{with tizen}
%endif

# Below features are used for unittest.
# Do not add neural network dependency to ML-Agent.
%define		release_test 0
%define		test_script $(pwd)/packaging/run_unittests.sh

# To generage gcov package, --define "gcov 1"
%if 0%{?gcov:1}
%define		unit_test 1
%define		release_test 1
%define		testcoverage 1
%endif
###########################################################################
# Disable a few features for TV release
%if "%{?profile}" == "tv"
%endif

# Disable a few features for DA release
%if 0%{?_with_da_profile}
%endif

# If it is tizen, we can export Tizen API packages.
%if %{with tizen}
%bcond_with tizen
%endif

# Note that debug packages generate an additional build and storage cost.
# If you do not need debug packages, run '$ gbs build ... --define "_skip_debug_rpm 1"'.
%if "%{?_skip_debug_rpm}" == "1"
%global debug_package %{nil}
%global __debug_install_post %{nil}
%endif
###########################################################################
# Package / sub-package definitions
Name:		machine-learning-agent
Summary:	AI service agent for ML(Machine Learning) API and NNStreamer
# Synchronize the version information for ML-Agent.
# 1. Tizen   : ./packaging/machine-learning-agent.spec
# 2. Meson   : ./meson.build
Version:	1.8.5
Release:	0
Group:		Machine Learning/ML Framework
Packager:	Sangjung Woo <sangjung.woo@samsung.com>
License:	Apache-2.0
Source0:	machine-learning-agent-%{version}.tar
Source1001:	machine-learning-agent.manifest

## Define build requirements ##
Requires:	libmachine-learning-agent = %{version}-%{release}

BuildRequires:	meson >= 0.50.0
BuildRequires:	glib2-devel
BuildRequires:	gstreamer-devel
BuildRequires:	pkgconfig(libsystemd)
BuildRequires:	pkgconfig(sqlite3)
BuildRequires:	pkgconfig(json-glib-1.0)
BuildRequires:	dbus

%if %{with tizen}
BuildRequires:	pkgconfig(dlog)
BuildRequires:	pkgconfig(libtzplatform-config)
BuildRequires:	pkgconfig(capi-appfw-package-manager)
BuildRequires:	pkgconfig(capi-appfw-app-common)
%endif # tizen

# For test
%if 0%{?unit_test}
BuildRequires:	pkgconfig(gtest)

%if 0%{?testcoverage}
# to be compatible with gcc-9, lcov should have a higher version than 1.14.1
BuildRequires:	lcov
%endif
%endif # unit_test

%description
AI service agent for ML(Machine Learning) API and NNStreamer.

%package -n libmachine-learning-agent
Summary:	Library that exports interfaces provided by Machine Learning Agent
Group:		Machine Learning/ML Framework = %{version}-%{release}
%description -n libmachine-learning-agent
Shared library to export interfaces provided by the Machine Learning Agent.

%package -n libmachine-learning-agent-devel
Summary:	Development headers and static library for interfaces provided by Machine Learning Agent
Group:		Machine Learning/ML Framework
Requires:	libmachine-learning-agent = %{version}-%{release}
%description -n libmachine-learning-agent-devel
Development headers and static library for interfaces provided by Machine Learning Agent.

%if 0%{?unit_test}
%if 0%{?release_test}
%package unittests
Summary:	Unittests for Machine Learning Agent
Group:		Machine Learning/ML Framework
Requires:	machine-learning-agent = %{version}-%{release}
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
###########################################################################
# Define build options
%define enable_tizen -Denable-tizen=false
%define service_db_path ""
%define service_db_key_prefix %{nil}
%define enable_gcov -Denable-gcov=false

# To set prefix, use this line
### define service_db_key_prefix -Dservice-db-key-prefix='some-prefix'

%if %{with tizen}
%define enable_tizen -Denable-tizen=true
%define service_db_path -Dservice-db-path=%{TZ_SYS_GLOBALUSER_DB}
%endif # tizen

%if 0%{?gcov}
%define enable_gcov -Denable-gcov=true
%endif

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
%else
%define install_test -Dinstall-test=false
%endif

%if 0%{?testcoverage}
# To test coverage, disable optimizations (and should unset _FORTIFY_SOURCE to use -O0)
CFLAGS=`echo $CFLAGS | sed -e "s|-O[1-9]|-O0|g"`
CFLAGS=`echo $CFLAGS | sed -e "s|-Wp,-D_FORTIFY_SOURCE=[1-9]||g"`
CXXFLAGS=`echo $CXXFLAGS | sed -e "s|-O[1-9]|-O0|g"`
CXXFLAGS=`echo $CXXFLAGS | sed -e "s|-Wp,-D_FORTIFY_SOURCE=[1-9]||g"`
# also, use the meson's base option, -Db_coverage, instead of --coverage/-fprofile-arcs and -ftest-coverage
%define enable_test_coverage -Db_coverage=true
%else
%define enable_test_coverage -Db_coverage=false
%endif

%else # unit_test
%define enable_test -Denable-test=false
%define install_test -Dinstall-test=false
%define enable_test_coverage -Db_coverage=false
%endif # unit_test

mkdir -p build

meson --buildtype=plain --prefix=%{_prefix} --sysconfdir=%{_sysconfdir} --libdir=%{_libdir} \
	--bindir=%{_bindir} --includedir=%{_includedir} \
	%{enable_test} %{install_test} %{enable_test_coverage} %{enable_gcov} \
	%{enable_tizen} %{service_db_path} %{service_db_key_prefix} \
	build

ninja -C build %{?_smp_mflags}

export MLAGENT_SOURCE_ROOT_PATH=$(pwd)
export MLAGENT_BUILD_ROOT_PATH=$(pwd)/%{builddir}

# Run test
# If gcov package generation is enabled, pass the test from GBS.
%if 0%{?unit_test} && !0%{?gcov}
bash %{test_script} ./tests/daemon/unittest_ml_agent
bash %{test_script} ./tests/daemon/unittest_service_db
bash %{test_script} ./tests/daemon/unittest_gdbus_util
%endif # unit_test

%install
DESTDIR=%{buildroot} ninja -C build %{?_smp_mflags} install

%if 0%{?unit_test}
%if 0%{?testcoverage}
# 'lcov' generates the date format with UTC time zone by default. Let's replace UTC with KST.
# If you can get a root privilege, run ln -sf /usr/share/zoneinfo/Asia/Seoul /etc/localtime
TZ='Asia/Seoul'; export TZ

# Get commit info
VCS=`cat ${RPM_SOURCE_DIR}/machine-learning-agent.spec | grep "^VCS:" | sed "s|VCS:\\W*\\(.*\\)|\\1|"`

# Create human readable coverage report web page.
# Create null gcda files if gcov didn't create it because there is completely no unit test for them.
find . -name "*.gcno" -exec sh -c 'touch -a "${1%.gcno}.gcda"' _ {} \;
# Remove gcda for meaningless file (CMake's autogenerated)
find . -name "CMakeCCompilerId*.gcda" -delete
find . -name "CMakeCXXCompilerId*.gcda" -delete
# Generate report
# TODO: the --no-external option is removed to include machine-learning-agent related source files.
# Restore this option when there is proper way to include those source files.
pushd build
lcov -t 'ML-Agent unittest coverage' -o unittest.info -c -d . -b $(pwd)
# Exclude generated files (e.g., Orc, Protobuf) and device-dependent files.
# Exclude files which are generated by gdbus-codegen and external files in /usr/*.
lcov -r unittest.info "*/tests/*" "*/meson*/*" "*/*@sha/*" "*/*.so.p/*" "*/*tizen*" "*/*-dbus.c" "/usr/*" -o unittest-filtered.info
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
%manifest machine-learning-agent.manifest
%license LICENSE
%attr(0755,root,root) %{_bindir}/machine-learning-agent
%attr(0644,root,root) %{_unitdir}/machine-learning-agent.service
%attr(0644,root,root) %config %{_sysconfdir}/dbus-1/system.d/machine-learning-agent.conf
%attr(0644,root,root) %{_datadir}/dbus-1/system-services/org.tizen.machinelearning.service.service

%files -n libmachine-learning-agent
%manifest machine-learning-agent.manifest
%license LICENSE
%{_libdir}/libml-agent.so.*

%files -n libmachine-learning-agent-devel
%manifest machine-learning-agent.manifest
%{_libdir}/libml-agent.so
%{_libdir}/libml-agent.a
%{_includedir}/ml-agent/ml-agent-interface.h
%{_libdir}/pkgconfig/ml-agent.pc

%if 0%{?unit_test}
%if 0%{?release_test}
%files unittests
%manifest machine-learning-agent.manifest
%{_bindir}/unittest-ml
%{_libdir}/libml-agent-test.a
%{_libdir}/libml-agent-test.so*
%if 0%{?gcov:1}
%{_bindir}/tizen-unittests/%{name}/run-unittest.sh
%endif
%endif # release_test

%if 0%{?gcov:1}
%files gcov
%{_datadir}/gcov/obj/*
%endif

%if 0%{?testcoverage}
%files unittest-coverage
%{_datadir}/ml-agent/unittest/*
%endif
%endif #unit_test

%changelog
* Mon Dec 11 2023 Sangjung Woo <sangjung.woo@samsung.com>
- Started ML-Agent packaging for 1.8.5 (Initial version sync to ML API)
