Name:           c-rbtree
Version:        1
Release:        2
Summary:        Red-Black Tree Implementation
License:        LGPL2+
URL:            https://github.com/c-util/c-rbtree
Source0:        %{name}.tar.xz
BuildRequires:  autoconf automake pkgconfig

%description
Standalone Red-Black Tree Implementation in Standard ISO-C11

%package        devel
Summary:        Development files for %{name}
Requires:       %{name} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%prep
%setup -q

%build
./autogen.sh
%configure
make %{?_smp_mflags}

%install
%make_install

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%doc COPYING
%{_libdir}/libcrbtree.so.*

%files devel
%{_includedir}/c-rbtree.h
%{_libdir}/libcrbtree.so
%{_libdir}/pkgconfig/c-rbtree.pc

%changelog
* Fri Apr 29 2016 <kay@redhat.com> 1-2
- update public header

* Mon Apr 25 2016 <kay@redhat.com> 1-1
- intial release
