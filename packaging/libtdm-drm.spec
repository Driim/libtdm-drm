Name:           libtdm-drm
Version:        1.0.8
Release:        0
Summary:        Tizen Display Manager DRM Back-End Library
Group:          Development/Libraries
License:        MIT
Source0:        %{name}-%{version}.tar.gz
Source1001:	    %{name}.manifest
BuildRequires: pkgconfig(libdrm)
BuildRequires: pkgconfig(libudev)
BuildRequires: pkgconfig(libtdm)
BuildRequires: pkgconfig(pixman-1)

%description
Back-End library of Tizen Display Manager DRM : libtdm-mgr DRM library

%global TZ_SYS_RO_SHARE  %{?TZ_SYS_RO_SHARE:%TZ_SYS_RO_SHARE}%{!?TZ_SYS_RO_SHARE:/usr/share}

%prep
%setup -q
cp %{SOURCE1001} .

%build
%reconfigure --prefix=%{_prefix} --libdir=%{_libdir}  --disable-static \
             CFLAGS="${CFLAGS} -Wall -Werror" \
             LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--as-needed"
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

%post
if [ -f %{_libdir}/tdm/libtdm-default.so ]; then
    rm -rf %{_libdir}/tdm/libtdm-default.so
fi
ln -s libtdm-drm.so %{_libdir}/tdm/libtdm-default.so

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%manifest %{name}.manifest
%license COPYING
%{_libdir}/tdm/libtdm-drm.so
