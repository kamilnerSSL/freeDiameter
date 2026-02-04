%global _build_id_links none

%define autorelease(e:s:pb:n) %{?-p:0.}%{lua:
    release_number = 7;
    base_release_number = tonumber(rpm.expand("%{?-b*}%{!?-b:1}"));
    print(release_number + base_release_number - 1);
}%{?-e:.%{-e*}}%{?-s:.%{-s*}}%{!?-n:%{?dist}}

%global _hardened_build 1
%define customversion .ringer

Name:           freeDiameter
Version:        1.6.1
Release:        %autorelease%{?customversion}
Summary:        A Diameter protocol open implementation
Requires:       kernel-modules-extra

License:        BSD-3-Clause
URL:            http://www.freediameter.net/
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  bison
BuildRequires:  cmake
BuildRequires:  flex
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  gnutls-devel
BuildRequires:  libgcrypt-devel
BuildRequires:  libidn-devel
BuildRequires:  lksctp-tools-devel
BuildRequires:  libmicrohttpd-devel
BuildRequires:  libxml2-devel
BuildRequires:  chrpath
BuildRequires:  systemd-rpm-macros

%description
freeDiameter is an open source Diameter protocol implementation. It provides an
extensible platform for deploying a Diameter network for your Authentication,
Authorization and Accounting needs.

%package devel
Summary:        Development files for freeDiameter
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
The %{name}-devel package contains the header files and libraries
necessary for developing extensions for %{name}.

%prep
%autosetup

%build
export CMAKE_SKIP_RPATH=TRUE

%cmake \
    -DCMAKE_INSTALL_PREFIX=%{_prefix} \
    -DINSTALL_LIB_DIR:PATH=%{_lib} \
    -DINSTALL_LIBRARY_SUFFIX:PATH=%{_lib} \
    -DINSTALL_EXTENSIONS_DIR:PATH=%{_lib}/freeDiameter \
    -DINSTALL_INCLUDE_DIR:PATH=include/freeDiameter \
    -DCMAKE_BUILD_TYPE=None \
    -DBUILD_RT_EREG:BOOL=ON \
    -DBUILD_DICT_LEGACY_XML:BOOL=ON \
    -DBUILD_DICT_DCCA_3GPP:BOOL=ON \
    -DBUILD_DICT_S6A:BOOL=ON \
    -DBUILD_DBG_METRICS:BOOL=ON \
    -Wno-dev
%cmake_build

%install
%cmake_install

if [ -d %{buildroot}/home ]; then
    echo "Fixing nested absolute paths..."
    # Find libraries in the nested path and move them to %{_libdir}
    find %{buildroot}/home -name "*.so*" -exec mv {} %{buildroot}%{_libdir}/ \;
    # Clean up the empty nested directories
    rm -rf %{buildroot}/home
fi

if [ ! -f %{buildroot}%{_unitdir}/freeDiameter.service ]; then
    mkdir -p %{buildroot}%{_unitdir}
    # Check etc/systemd/system or usr/lib/systemd/system where it might land
    if [ -f %{buildroot}%{_sysconfdir}/systemd/system/freeDiameter.service ]; then
        mv %{buildroot}%{_sysconfdir}/systemd/system/freeDiameter.service %{buildroot}%{_unitdir}/
    elif [ -f %{buildroot}/usr/lib/systemd/system/freeDiameter.service ]; then
        mv %{buildroot}/usr/lib/systemd/system/freeDiameter.service %{buildroot}%{_unitdir}/
    else
        # Fallback manual install from source tree
        install -m 0644 contrib/RPM/freeDiameter.service %{buildroot}%{_unitdir}/freeDiameter.service
    fi
fi

if [ ! -f %{buildroot}%{_sysconfdir}/rsyslog.d/40-freeDiameter.conf ]; then
    mkdir -p %{buildroot}%{_sysconfdir}/rsyslog.d
    install -m 0644 contrib/RPM/rsyslog-freeDiameter.conf %{buildroot}%{_sysconfdir}/rsyslog.d/40-freeDiameter.conf
fi

mkdir -p %{buildroot}%{_localstatedir}/log/freeDiameter

find %{buildroot}%{_libdir} -type f -name "*.so*" -exec chrpath --delete {} \;
find %{buildroot}%{_libdir}/freeDiameter/ -type f -name "*.fdx" -exec chrpath --delete {} \;

if [ "%{_lib}" = "lib64" ] && [ -d %{buildroot}/usr/lib/freeDiameter ]; then
    rm -rf %{buildroot}/usr/lib/freeDiameter
fi

mkdir -p %{buildroot}%{_sysconfdir}/freeDiameter
install -m 0644 doc/freediameter.conf.sample %{buildroot}%{_sysconfdir}/freeDiameter/freeDiameter.conf.sample

%post
%systemd_post freeDiameter.service
/sbin/ldconfig

%preun
%systemd_preun freeDiameter.service

%postun
%systemd_postun_with_restart freeDiameter.service
/sbin/ldconfig

%files
%license LICENSE
%doc README NEWS doc/*.sample
%{_bindir}/freeDiameterd*
%{_libdir}/libfdcore.so.*
%{_libdir}/libfdproto.so.*
%dir %{_libdir}/freeDiameter
%{_libdir}/freeDiameter/*.fdx
%dir %{_sysconfdir}/freeDiameter
%config(noreplace) %{_sysconfdir}/freeDiameter/freeDiameter.conf.sample
%{_unitdir}/freeDiameter.service
%config(noreplace) %{_sysconfdir}/rsyslog.d/40-freeDiameter.conf
%dir %{_localstatedir}/log/freeDiameter

%files devel
# Since freeDiameter.h is likely INSIDE the directory, we just include the directory
%{_includedir}/freeDiameter/
%{_libdir}/libfdcore.so
%{_libdir}/libfdproto.so

%changelog
* Wed Feb 4 2026 Keith Milner <kamilner@sslconsult.com> - 1.6.1-7
- Updates to DICT_S6A
* Sun Feb 1 2026 Keith Milner <kamilner@sslconsult.com> - 1.6.1-6
- Added DICT_S6A to build
* Fri Jan 30 2026 Keith Milner <kamilner@sslconsult.com> - 1.6.1-5
- Added DICT_DCCA_3GPP and DICT_LEGACY_XML to build
* Thu Jan 22 2026 Keith Milner <kamilner@sslconsult.com> - 1.6.1-4
- Enhanced dbg_metrics extension with more data and config.
* Thu Jan 22 2026 Keith Milner <kamilner@sslconsult.com> - 1.6.1-3
- Enhanced dbg_metrics extension.
* Thu Jan 22 2026 Keith Milner <kamilner@sslconsult.com> - 1.6.1-2
- Added dbg_metrics extension.
* Wed Jan 22 2025 Keith Milner <kamilner@sslconsult.com> - 1.6.1-1
- Updated to v 1.6.1
- Added Fixup section to handle nested absolute paths in BUILDROOT.
- Forced libraries and extensions into %%{_libdir} via CMake PATH flags.
* Fri May 31 2024 Keith Milner <kamilner@sslconsult.com> - 1.6.0-5
- Fix binary and library filenames in %%files to match project restructuring.
- Use %%{_libdir} for all library and extension paths.
- Enable rt_ereg extension compilation.
* Mon May 20 2024 Keith Milner <kamilner@sslconsult.com> - 1.6.0-4
- Add rsyslog files.
* Mon May 20 2024 Keith Milner <kamilner@sslconsult.com> - 1.6.0-3
- Add kernel-modules-extra as runtime dependency.
* Thu May 16 2024 Keith Milner <kamilner@sslconsult.com> - 1.6.0-2
- Add custom version extension.
* Wed May 15 2024 Keith Milner <kamilner@sslconsult.com> - 1.6.0-1
- Initial RPM release for freeDiameter.
