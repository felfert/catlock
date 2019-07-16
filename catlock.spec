%define ver 1.0
%define rel 1%{?dist}

Name:		    catlock
Summary:	    Lock your computer against cat access
Version:	    %{ver}
Release:	    %{rel}
License:	    BSD-2-Clause
Group:		    Applications/X11
URL:		    https://github.com/felfert/catlock
Source0:	    https://github.com/felfert/catlock/archive/%{name}-%{version}.tar.gz
BuildRequires:  cmake >= 3.10
BuildRequires:  gcc-c++ pkg-config xmlto make desktop-file-utils
BuildRequires:  libxcb-devel xcb-util-keysyms-devel xcb-util-image-devel
BuildRequires:  libX11-devel popt-devel

%description
catlock locks the keyboard and mouse against cats, walking over them.

%prep
%autosetup

%build
%cmake
%make_build

%install
%make_install
desktop-file-validate %{buildroot}/%{_datadir}/applications/%{name}.desktop

%post
update-desktop-database

%postun
update-desktop-database

%files
%doc CODE_OF_CONDUCT.md  LICENSE  README.md
%doc %{_mandir}/man1/%{name}.1*
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.png

%changelog
* Mon Jul  8 2019 Fritz Elfert <catlock@fritz-elfert.de> - 1.0-1
- Initial release.
