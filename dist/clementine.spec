Name:           clementine
Version:        0.2
Release:        5%{?dist}
Summary:        A music player and library organiser

Group:          Applications/Multimedia
License:        GPLv3
URL:            http://code.google.com/p/clementine-player
Source0:        %{name}-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  desktop-file-utils liblastfm-devel taglib-devel xine-lib-devel
BuildRequires:  libnotify-devel qt4-devel boost-devel notification-daemon gcc-c++
Requires:       xine-lib-extras-freeworld

%description
Clementine is a modern music player and library organiser.
It is largely a port of Amarok 1.4, with some features rewritten to take
advantage of Qt4.

%prep
%setup -q


%build
qmake-qt4
make %{?_smp_mflags}


%install
install -d %{buildroot}/%{_bindir}
install -d %{buildroot}%{_datadir}/applications
install -d %{buildroot}%{_datadir}/icons/hicolor/64x64/apps

install -m 0755 src/%{name} %{buildroot}/%{_bindir}/clementine
install -m 0644 dist/%{name}.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop
install -m 0644 dist/%{name}_64.png %{buildroot}/%{_datadir}/icons/hicolor/64x64/apps/application-x-clementine.png

desktop-file-install \
    --dir %{buildroot}%{_datadir}/applications \
    --delete-original \
    %{buildroot}%{_datadir}/applications/%{name}.desktop

%clean
make clean
rm Makefile src/Makefile


%files
%defattr(-,root,root,-)
%doc
%{_bindir}/clementine
%{_datadir}/applications/clementine.desktop
%{_datadir}/icons/hicolor/64x64/apps/application-x-clementine.png


%changelog
* Mon Mar 22 2010 David Sansome <me@davidsansome.com> - 0.2
- Version 0.2

* Sun Feb 21 2010 David Sansome <me@davidsansome.com> - 0.1-5
- Various last-minute bugfixes

* Sun Jan 17 2010 David Sansome <me@davidsansome.com> - 0.1-1
- Initial package
