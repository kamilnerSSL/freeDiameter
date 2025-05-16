%define autorelease(e:s:pb:n) %{?-p:0.}%{lua:
    release_number = 4;
    base_release_number = tonumber(rpm.expand("%{?-b*}%{!?-b:1}"));
    print(release_number + base_release_number - 1);
}%{?-e:.%{-e*}}%{?-s:.%{-s*}}%{!?-n:%{?dist}}

%global _hardened_build 1
%define customversion .custom # Define custom version macro

Name:           freeDiameter
Version:        1.6.0
Release:        %autorelease%{?customversion}
Summary:        A Diameter protocol open implementation
Requires:       kernel-modules-extra

License:        BSD-3-Clause
URL:            http://www.freediameter.net/
Source0:        https://github.com/%{name}/%{name}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  bison
BuildRequires:  cmake
BuildRequires:  flex
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  gnutls-devel
BuildRequires:  libgcrypt-devel
BuildRequires:  libidn-devel
BuildRequires:  lksctp-tools-devel
BuildRequires:  chrpath

%description
freeDiameter is an open source Diameter protocol implementation. It provides an
extensible platform for deploying a Diameter network for your Authentication,
Authorization and Accounting needs.

%package devel
Summary:        Library for freeDiameter package
Requires:       %{name} = %{version}-%{release}

%description devel
The %{name}-devel package contains the shared library
for %{name} package.

%prep
%autosetup

%build
# Set rpath to ""
export CMAKE_SKIP_RPATH=TRUE
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DCMAKE_BUILD_TYPE=None . -Wno-dev
%cmake_build

%install
%cmake_install
#  Install additional files
mkdir -p %{buildroot}/etc/freeDiameter
install -m 0644 doc/freediameter.conf.sample %{buildroot}/etc/freeDiameter/freeDiameter.conf.sample
mkdir -p %{buildroot}/etc/systemd/system
install -m 0644 contrib/RPM/freeDiameter.service %{buildroot}/etc/systemd/system/freeDiameter.service
# Install extensions
mkdir -p %{buildroot}/usr/lib/freeDiameter
install -m 0644 redhat-linux-build/extensions/*.fdx %{buildroot}/usr/lib/freeDiameter/
# Remove rpaths from fdx files
find %{buildroot}/usr/lib/freeDiameter/ -type f -exec chrpath --delete {} \;
install -m 0644 contrib/RPM/rsyslog-freeDiameter.conf %{buildroot}/etc/rsyslog.d/40-freeDiameter.conf
# Create logging directory
mkdir -p %{buildroot}/var/log/freeDiameter

%post
systemctl daemon-reload
%ldconfig_scriptlets

%files
%doc doc
%{_bindir}/freeDiameterd
%{_bindir}/%{name}d-%{version}
%{_libdir}/libfdcore.so.7
%{_libdir}/libfdproto.so.7
%{_libdir}/libfdcore.so.%{version}
%{_libdir}/libfdproto.so.%{version}
/etc/freeDiameter/freeDiameter.conf.sample
/etc/systemd/system/freeDiameter.service
/etc/rsyslog.d/40-freeDiameter.conf
/usr/lib/freeDiameter/*.fdx
/var/log/freeDiameter

%files devel
%{_includedir}/%{name}/
%{_libdir}/%{name}/
%{_libdir}/libfdcore.so
%{_libdir}/libfdproto.so

%changelog
* Tue May 20 2025 Keith Milner <kamilner@sslconsult.com> - 1.6.0-4
- Add rsyslog files
+
* Tue May 20 2025 Keith Milner <kamilner@sslconsult.com> - 1.6.0-3
- Add kernel-modules-extra as runtime dependency
+
* Fri May 16 2025 Keith Milner <kamilner@sslconsult.com> - 1.6.0-2
- Add custom version extension.
+
* Thu May 15 2025 Keith Milner <kamilner@sslconsult.com> - 1.6.0-1
- Initial RPM release for freeDiameter.
