Name:           c-rbtree
Version:        1
Release:        4%{?dist}
Summary:        Red-Black Tree Implementation
License:        LGPLv2+
URL:            https://github.com/c-util/c-rbtree
Source0:        https://github.com/c-util/c-rbtree/archive/v%{version}.tar.gz
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

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%license COPYING
%license LICENSE.LGPL2.1
%{_libdir}/libcrbtree.so.*

%files devel
%{_includedir}/c-rbtree.h
%{_libdir}/libcrbtree.so
%{_libdir}/pkgconfig/c-rbtree.pc

%changelog
* Tue Jun 21 2016 <kay@redhat.com> 1-4
- update spec file according to Fedora guidelines

* Sun May 08 2016 <daherrma@redhat.com> 1-3
- add postorder traversal

* Fri Apr 29 2016 <kay@redhat.com> 1-2
- update public header

* Mon Apr 25 2016 <kay@redhat.com> 1-1
- intial release
