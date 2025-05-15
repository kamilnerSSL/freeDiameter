Name:           freeDiameter
Version:        1.5.0
Release:        1%{?dist}
Summary:        Open source Diameter protocol implementation

License:        BSD
URL:            https://github.com/freeDiameter/freeDiameter
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  gnutls-devel
BuildRequires:  libidn-devel
BuildRequires:  libtool
BuildRequires:  pkgconfig
BuildRequires:  json-c-devel
BuildRequires:  sqlite-devel
BuildRequires:  libpcap-devel

Requires:       gnutls
Requires:       json-c
Requires:       sqlite

# Define install prefix with conditional
%global install_prefix %{?_with_local_prefix:/usr/local}%{!?_with_local_prefix:/usr}

%description
freeDiameter is an open-source Diameter protocol implementation. It is modular, extensible,
and aims to conform to relevant IETF and 3GPP specifications for AAA protocols.

%prep
%autosetup

%build
mkdir -p build
cd build
%cmake .. -DCMAKE_INSTALL_PREFIX=%{install_prefix}
%cmake_build

%install
cd build
%cmake_install

%files
%license COPYING
%doc README.md
%{install_prefix}/bin/*
%{install_prefix}/lib*/libfd*.so*
%{install_prefix}/lib*/libfd*.a
%{install_prefix}/include/freeDiameter/
%{install_prefix}/lib*/pkgconfig/freeDiameter.pc
%{install_prefix}/share/freeDiameter/

%changelog
* Fri May 09 2025 Keith Milner <kamilner@sslconsult.com> - 1.5.0-1
- RPM packaging with dynamic install prefix support via --with local_prefix

