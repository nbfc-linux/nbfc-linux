# originally taken from https://copr-dist-git.fedorainfracloud.org/cgit/trixieua/nbfc-linux/nbfc-linux.git/tree/
%define _disable_source_fetch 0
%global toolchain clang
%global debug_package %{nil}

Name:           nbfc-linux
Version:        0.1.15
Release:        1%{?dist}
Summary:        NoteBook FanControl

License:        GPL-3.0
URL:            https://github.com/nbfc-linux/nbfc-linux
Source0:        https://github.com/nbfc-linux/nbfc-linux/archive/refs/tags/%{version}.tar.gz

BuildRequires:  clang autoconf
Requires:  python3

%description
NoteBook FanControl

%prep
%autosetup

%build
# ./autogen.sh
# ./configure
%make_build


%install
%make_install
cp -r %{buildroot}/usr/local/* %{buildroot}/usr
cp -r %{buildroot}/usr/etc %{buildroot}
rm -rf %{buildroot}/usr/local

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
