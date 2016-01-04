Name:       iotcon
Summary:    Tizen IoT Connectivity
Version:    0.0.1
Release:    0
Group:      Network & Connectivity/Service
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    %{name}.service
Source1001: %{name}.manifest
Source1002: %{name}-old.manifest
Source1003: %{name}-test-old.manifest
Source2001: %{name}.conf.in
BuildRequires:  cmake
BuildRequires:  boost-devel
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(iotivity)
%if 0%{?tizen_version_major} >= 3
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-session)
BuildRequires:  pkgconfig(cynara-creds-gdbus)
%endif
%if "%{tizen}" == "2.3"
BuildRequires:  python-xml
%endif

%define _unitdir /usr/lib/systemd/system

%description
Tizen IoT Connectivity Service & Library(Client) based on Iotivity


%package devel
Summary:    TizenIoT Connectivity(devel)
Group:      Network & Connectivity/Development
Requires:   %{name} = %{version}

%description devel
IoT Connectivity Manager development Kit


%package test
Summary:    Tizen IoT Connectivity(test)
Group:      Network & Connectivity/Testing
Requires:   %{name} = %{version}

%description test
Tizen IoT Connectivity Test Programs


%prep
%setup -q
chmod g-w %_sourcedir/*
%if 0%{?tizen_version_major} < 3
cp %{SOURCE1002} ./%{name}.manifest
cp %{SOURCE1003} ./%{name}-test.manifest
%else
cp %{SOURCE1001} ./%{name}.manifest
cp %{SOURCE1001} ./%{name}-test.manifest
cp %{SOURCE2001} .
%endif


%build
%if 0%{?tizen_version_major} < 3
TZ_VER_3=0
%else
TZ_VER_3=1
%endif

MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DMAJORVER=${MAJORVER} -DFULLVER=%{version} -DBIN_INSTALL_DIR:PATH=%{_bindir} \
		-DTZ_VER_3=${TZ_VER_3}


%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_unitdir}/multi-user.target.wants
cp -af %{SOURCE1} %{buildroot}%{_unitdir}/
ln -s ../%{name}.service %{buildroot}%{_unitdir}/multi-user.target.wants/%{name}.service

%if 0%{?tizen_version_major} < 3
mkdir -p %{buildroot}/%{_datadir}/license
cp LICENSE.APLv2 %{buildroot}/%{_datadir}/license/%{name}
cp LICENSE.APLv2 %{buildroot}/%{_datadir}/license/%{name}-test
%endif


%post
systemctl daemon-reload
if [ $1 == 1 ]; then
    systemctl restart %{name}.service
fi
/sbin/ldconfig

%postun
if [ $1 == 0 ]; then
    systemctl stop %{name}.service
fi
systemctl daemon-reload
/sbin/ldconfig


%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_bindir}/%{name}-daemon
%{_libdir}/lib%{name}.so.*
%{_unitdir}/%{name}.service
%{_unitdir}/multi-user.target.wants/%{name}.service
%if 0%{?tizen_version_major} < 3
%{_datadir}/license/%{name}
%else
%config %{_sysconfdir}/dbus-1/system.d/%{name}.conf
%license LICENSE.APLv2
%endif

%files devel
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/%{name}/*.h

%files test
%manifest %{name}-test.manifest
%defattr(-,root,root,-)
%{_bindir}/iotcon-test-*
%if 0%{?tizen_version_major} < 3
%{_datadir}/license/%{name}-test
%else
%license LICENSE.APLv2
%endif
