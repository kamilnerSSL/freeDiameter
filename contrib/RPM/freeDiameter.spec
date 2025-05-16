%define autorelease(e:s:pb:n) %{?-p:0.}%{lua:
    release_number = 2;
    base_release_number = tonumber(rpm.expand("%{?-b*}%{!?-b:1}"));
    print(release_number + base_release_number - 1);
}%{?-e:.%{-e*}}%{?-s:.%{-s*}}%{!?-n:%{?dist}}

%global _hardened_build 1
# Define custom version macro
%define customversion .ringer

Name:           freeDiameter
Version:        1.6.0
Release:        %autorelease%{?customversion}
Summary:        A Diameter protocol open implementation

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
/usr/lib/freeDiameter/*.fdx
Requires:       kernel-modules-extra

%files devel
%{_includedir}/%{name}/
%{_libdir}/%{name}/
%{_libdir}/libfdcore.so
%{_libdir}/libfdproto.so

%changelog
* Thu May 15 2025 Keith Milner <kamilner@sslconsult.com> - 1.6.0-1
- Initial RPM release for freeDiameter.
+
* Fri May 16 2025 Keith Milner <kamilner@sslconsult.com> - 1.6.0-2
- Add custom version extension.
