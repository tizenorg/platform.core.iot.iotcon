Name:       iotcon
Summary:    IoT Connectivity Manager
Version:    0.0.1
Release:    0
Group:      Network & Connectivity/Other
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1001: %{name}.manifest
BuildRequires:  cmake
BuildRequires:  boost-devel
BuildRequires:  iotivity-devel
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(json-glib-1.0)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(notification)


%description
IoT Connectivity Manager and Library

%package devel
Summary:    IoT Connectivity Manager (devel)
Group:      Network & Connectivity/Development
Requires:   %{name} = %{version}

%description devel
IoT Connectivity Manager development Kit

%prep
%setup -q
cp %{SOURCE1001} .

%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DMAJORVER=${MAJORVER} -DFULLVER=%{version} -DBIN_INSTALL_DIR:PATH=%{_bindir}


%install
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so.*
%license LICENSE.APLv2
%{_bindir}/crud-test-client
%{_bindir}/crud-test-server

%files devel
%{_libdir}/lib%{name}.so
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/%{name}/*.h

