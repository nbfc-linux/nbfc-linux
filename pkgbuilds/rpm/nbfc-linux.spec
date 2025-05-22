# originally taken from https://copr-dist-git.fedorainfracloud.org/cgit/trixieua/nbfc-linux/nbfc-linux.git/tree/
%define _disable_source_fetch 0
%global toolchain clang
%global debug_package %{nil}

Name:           nbfc-linux
Version:        0.3.17
Release:        1%{?dist}
Summary:        NoteBook FanControl

License:        GPL-3.0
URL:            https://github.com/nbfc-linux/nbfc-linux
Source0:        https://github.com/nbfc-linux/nbfc-linux/archive/refs/tags/%{version}.tar.gz

BuildRequires:  clang autoconf
Requires:  libcurl

%description
NoteBook FanControl

%prep
%autosetup

%build
libtoolize --force
aclocal
autoheader || true
automake --force-missing --add-missing || true
autoconf
./configure --prefix=/usr --sysconfdir=/etc
%make_build


%install
%make_install
mkdir -p %{buildroot}/usr/bin/
cp -r %{buildroot}/bin/* %{buildroot}/usr/bin/
rm -rf %{buildroot}/usr/local
rm -rf %{buildroot}/bin

%post
%systemd_post nbfc_service.service

%preun
%systemd_preun nbfc_service.service

%postun
%systemd_postun nbfc_service.service

%files
%{_bindir}/*
%{_datadir}/*
/etc/nbfc/
%{_usr}/lib/systemd/system/nbfc_service.service
