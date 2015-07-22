Name:       iotcon
Summary:    IoT Connectivity Manager
Version:    0.0.1
Release:    0
Group:      Network & Connectivity/Other
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    %{name}.service
Source1001: %{name}.manifest
Source1002: lib%{name}.manifest
BuildRequires:  cmake
BuildRequires:  boost-devel
BuildRequires:  iotivity-devel
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(json-glib-1.0)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-base-common)
%if "%{tizen}" == "2.3"
BuildRequires:  python-xml
%endif

%define _unitdir /usr/lib/systemd/system

%description
IoT Connectivity Manager Daemon


%package lib
Summary:    IoT Connectivity Library
Group:      Network & Connectivity/Libraries
Requires:   %{name} = %{version}

%description lib
Tizen event notification service Client library for applications.


%package devel
Summary:    IoT Connectivity Manager (devel)
Group:      Network & Connectivity/Development
Requires:   %{name}-lib = %{version}

%description devel
IoT Connectivity Manager development Kit


%package test
Summary:    IoT Connectivity Manager (test)
Group:      Network & Connectivity/Development
Requires:   %{name}-lib = %{version}

%description test
IoT Connectivity Manager Test Programs


%prep
%setup -q
cp %{SOURCE1001} .
cp %{SOURCE1002} .


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DMAJORVER=${MAJORVER} -DFULLVER=%{version} -DBIN_INSTALL_DIR:PATH=%{_bindir}


%install
rm -rf %{buildroot}/BUILD/%{name}*
%make_install

mkdir -p %{buildroot}%{_unitdir}/multi-user.target.wants
cp -af %{SOURCE1} %{buildroot}%{_unitdir}/
ln -s ../%{name}.service %{buildroot}%{_unitdir}/multi-user.target.wants/%{name}.service

%if 0%{?tizen_version_major} < 3
mkdir -p %{buildroot}/%{_datadir}/license
cp LICENSE.APLv2 %{buildroot}/%{_datadir}/license/%{name}
cp LICENSE.APLv2 %{buildroot}/%{_datadir}/license/%{name}-lib
%endif


%post
systemctl daemon-reload
if [ $1 == 1 ]; then
    systemctl restart %{name}.service
fi

%postun
/sbin/ldconfig
if [ $1 == 0 ]; then
    systemctl stop %{name}.service
fi
systemctl daemon-reload


%post lib -p /sbin/ldconfig
%postun lib -p /sbin/ldconfig


%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_unitdir}/%{name}.service
%{_unitdir}/multi-user.target.wants/%{name}.service
%{_bindir}/%{name}-daemon
%{_datadir}/dbus-1/services/org.tizen.%{name}.dbus.service
%if 0%{?tizen_version_major} < 3
%{_datadir}/license/%{name}
%else
%license LICENSE.APLv2
%endif

%files lib
%manifest lib%{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so.*
%if 0%{?tizen_version_major} < 3
%{_datadir}/license/%{name}-lib
%else
%license LICENSE.APLv2
%endif

%files devel
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/%{name}/*.h

%files test
%defattr(-,root,root,-)
%{_bindir}/crud-test-client
%{_bindir}/crud-test-server
%{_bindir}/device-test-client
%{_bindir}/device-test-server
%{_bindir}/repr-test-client
%{_bindir}/repr-test-server
