# Define the kmod package name here.
%define kmod_name TRACE

# If you want to use another kernel version, run this command:
#  rpmbuild -ba --define 'kversion 3.10.0-957.5.1.el7.x86_64' mykmod.spec
%{!?kversion: %define kversion 3.10.0-957.el7.%{_target_cpu}}

Name:    %{kmod_name}-kmod
Version: v3
Release: 1%{?dist}
Group:   System Environment/Kernel
License: GPLv2
Summary: %{kmod_name} kernel module(s)
URL:     https://cdcvs.fnal.gov/redmine/projects/trace

BuildRequires: redhat-rpm-config, perl, make, bash, gcc

# Sources.
Source0:  %{kmod_name}-%{version}.tar.bz2
Source10: kmodtool-%{kmod_name}.sh

%if 0%{?rhel} > 0
# Add note on any non whitelisted ABI symbols
BuildRequires: kernel-abi-whitelists
%endif

# Magic hidden here.
%{expand:%(sh %{SOURCE10} rpmtemplate %{kmod_name} %{kversion} "")}

# Disable the building of the debug package(s).
%define debug_package %{nil}

%description
This package provides the %{kmod_name} kernel module(s).

It controls tracing via environment variables and dynamically from
outside program.

It is built to depend upon the specific ABI provided by a range of releases
of the same variant of the Linux kernel and not on any one specific build.

###########################################################
%package -n %{kmod_name}-utils
Summary: Utilities for %{kmod_name} kmod
Requires: %{kmod_name}-kmod
Requires: perl, bash

%description -n %{kmod_name}-utils
Utilities for %{kmod_name} kmod

%files -n %{kmod_name}-utils
%defattr(0644,root,root,0755)
%doc %{_mandir}/man1/*
%doc %{_mandir}/man1p/*
%doc %{_defaultdocdir}/%{kmod_name}-utils-%{version}/*
%attr(0755,root,root) %{_bindir}/trace_delta
%{_sysconfdir}/profile.d/trace.sh
%{_includedir}/TRACE/*

###########################################################
%prep
# Prep kernel module
%setup -q -n %{kmod_name}-%{version}

## Write ABI tracking file
echo "override %{kmod_name} * weak-updates/%{kmod_name}" > kmod-%{kmod_name}.conf

###########################################################
%build
%{__mkdir} build
# Build all (TRACE packages its own implementation of module-build
%{__make} OUT=${PWD}/build all KDIR=%{_usrsrc}/kernels/%{kversion}

###########################################################
%install
# Install kernel module
%{__install} -d %{buildroot}/lib/modules/%{kversion}/extra/%{kmod_name}/
%{__install} build/module/%{kversion}/%{kmod_name}.ko %{buildroot}/lib/modules/%{kversion}/extra/%{kmod_name}/
%{__install} -d %{buildroot}%{_sysconfdir}/depmod.d/
%{__install} kmod-%{kmod_name}.conf %{buildroot}%{_sysconfdir}/depmod.d/

## Strip the modules(s).
find %{buildroot} -type f -name \*.ko -exec %{__strip} --strip-debug \{\} \;

## Sign the modules(s).
%if %{?_with_modsign:1}%{!?_with_modsign:0}
## If the module signing keys are not defined, define them here.
%{!?privkey: %define privkey %{_sysconfdir}/pki/SECURE-BOOT-KEY.priv}
%{!?pubkey: %define pubkey %{_sysconfdir}/pki/SECURE-BOOT-KEY.der}
for module in $(find %{buildroot} -type f -name \*.ko);
do %{__perl} /usr/src/kernels/%{kversion}/scripts/sign-file \
    sha256 %{privkey} %{pubkey} $module;
done
%endif

# Install headers
%{__install} -d %{buildroot}%{_includedir}/TRACE
%{__install} include/trace.h %{buildroot}%{_includedir}/TRACE/trace.h
%{__install} include/tracemf.h %{buildroot}%{_includedir}/TRACE/tracemf.h

# Install shell profile
%{__install} -d %{buildroot}%{_sysconfdir}/profile.d/
%{__install} build/script/trace.sh.functions %{buildroot}%{_sysconfdir}/profile.d/trace.sh

# Install utils
%{__install} -d %{buildroot}%{_bindir}
%{__install} build/script/trace_delta.pl %{buildroot}%{_bindir}/trace_delta

# Install manpages
%{__install} -d %{buildroot}%{_mandir}/man1
%{__install} doc/t*.1 %{buildroot}%{_mandir}/man1/
%{__install} -d %{buildroot}%{_mandir}/man1p
%{__install} doc/t*.1p %{buildroot}%{_mandir}/man1p/

# Install kmod docs
%{__install} -d %{buildroot}%{_defaultdocdir}/kmod-%{kmod_name}-%{version}/
%{__install} doc/users_guide.txt %{buildroot}%{_defaultdocdir}/kmod-%{kmod_name}-%{version}/
%{__install} -d %{buildroot}%{_defaultdocdir}/kmod-%{kmod_name}-%{version}/example_module
%{__cp} -r src_example/module %{buildroot}%{_defaultdocdir}/kmod-%{kmod_name}-%{version}/example_module/
%{__cp} -r src_example/module1 %{buildroot}%{_defaultdocdir}/kmod-%{kmod_name}-%{version}/example_module/

# Install util docs
%{__install} -d %{buildroot}%{_defaultdocdir}/%{kmod_name}-utils-%{version}/
%{__install} -d %{buildroot}%{_defaultdocdir}/%{kmod_name}-utils-%{version}/example_util
%{__cp} -r src_example/userspace %{buildroot}%{_defaultdocdir}/%{kmod_name}-utils-%{version}/example_util


###########################################################
%clean
%{__rm} -rf %{buildroot}

###########################################################
%changelog
