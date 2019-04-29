#!/bin/sh
# Define the kmod package name here.
%define kmod_name TRACE

# If you want to use another kernel version, run this command:
#  rpmbuild -ba --define 'kversion 3.10.0-957.5.1.el7.x86_64' mykmod.spec
# Default is current running kernel
%{!?kversion: %define kversion %(uname -r | sed -e "s/\.`uname -p`//").%{_target_cpu}}

# If you want a custom version:
# rpmbuild -ba --define 'trace_version myversion'
# If you want a custom revision:
# rpmbuild -ba --define 'trace_revision myrevision'

###########################################################
###########################################################

Name:    %{kmod_name}-kmod
Group:   System Environment/Kernel
License: GPLv2
Summary: %{kmod_name} kernel module(s)
URL:     https://cdcvs.fnal.gov/redmine/projects/trace
Packager:	Fermilab Real-Time Software Infrastructure
# Sources
Source0:  %{kmod_name}.tar.bz2
Source10: kmodtool-%{kmod_name}.sh

# Determine the UPS Version from source if not specified
### untar the source, ask and remove the untar'd copy
%if "x%{?trace_version}" == "x"
%define trace_version %(mkdir -p %{_builddir}/%{kmod_name}; cd  %{_builddir}/%{kmod_name} ; tar xf %{SOURCE0} ;\
                        grep 'parent TRACE ' ups/product_deps 2>/dev/null | grep -v '\#' | awk '{print $3}'; rm -rf %{_builddir}/%{kmod_name}) 
%endif

# Determine the svn revision from source if not specified
### untar the source, ask and remove the untar'd copy
%if "x%{?trace_revision}" == "x"
%define trace_revision r%(mkdir -p %{_builddir}/%{kmod_name}; cd  %{_builddir}/%{kmod_name} ; tar xf %{SOURCE0} ;\
                           svn info 2>/dev/null |grep '^Revision: ' | awk '{print $2}'; rm -rf %{_builddir}/%{kmod_name}) 
%endif

# Determine if source repo is clean
### untar the source, check and remove the untar'd copy
%define repo_clean %(mkdir -p %{_builddir}/%{kmod_name}; cd  %{_builddir}/%{kmod_name} ; tar xf %{SOURCE0} ;\
                     svn diff 2>&1 | wc -l; rm -rf %{_builddir}/%{kmod_name})
%if "%repo_clean" != "0"
%define unclean .WITHLOCALCHANGES
%endif

Version: %{trace_version}
# Add the ".1" as a place where you can increment a value to resolve a packaging issue
Release: %{?trace_revision}.1%{?dist}%{?unclean}

BuildRequires: redhat-rpm-config, perl, make, bash, gcc
BuildRequires: gawk, coreutils, sed, svn, gcc-c++
BuildRequires: kernel-devel = %(echo %{kversion} | sed -e 's/.%{_target_cpu}//')

%if 0%{?rhel} > 0
# Add note on any non whitelisted ABI symbols
BuildRequires: kernel-abi-whitelists
%if 0%{?rhel} > 6
ExcludeArch:	%{ix86}
%endif
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
%attr(0755,root,root) %{_bindir}/trace_cntl
%attr(0755,root,root) %{_bindir}/trace_delta
%attr(0755,root,root) %{_bindir}/bitN_to_mask
%attr(0755,root,root) %{_bindir}/trace_envvars
%{_sysconfdir}/profile.d/trace.sh
%{_includedir}/TRACE/*

###########################################################
%prep
# Prep kernel module
%setup -q -c -n %{kmod_name}

%{__mkdir_p} build
## Write ABI tracking file
echo "override %{kmod_name} * weak-updates/%{kmod_name}" > build/kmod-%{kmod_name}.conf

###########################################################
%build
%{__mkdir_p} build
# Build all (TRACE packages its own implementation of module-build)
#  when the distro has gcc 4.9+, (and std is less than c11) then add XTRA_CFLAGS=-std=c11
%{__make} OUT=${PWD}/build XTRA_CFLAGS=-D_GNU_SOURCE XTRA_CXXFLAGS=-std=c++11 all KDIR=%{_usrsrc}/kernels/%{kversion}

###########################################################
%install
rm -rf %{buildroot}
# Install kernel module
%{__install} -d %{buildroot}/lib/modules/%{kversion}/extra/%{kmod_name}/
%{__install} build/module/%{kversion}/%{kmod_name}.ko %{buildroot}/lib/modules/%{kversion}/extra/%{kmod_name}/
%{__install} -d %{buildroot}%{_sysconfdir}/depmod.d/
%{__install} -m644 build/kmod-%{kmod_name}.conf %{buildroot}%{_sysconfdir}/depmod.d/

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
%{__install} -m644 include/trace.h %{buildroot}%{_includedir}/TRACE/trace.h
%{__install} -m644 include/tracemf.h %{buildroot}%{_includedir}/TRACE/tracemf.h

# Install shell profile
%{__install} -d %{buildroot}%{_sysconfdir}/profile.d/
%{__install} script/trace_functions.sh %{buildroot}%{_sysconfdir}/profile.d/trace.sh

# Install utils
%{__install} -d %{buildroot}%{_bindir}
%{__install} %(echo build/Linux*/bin/trace_cntl) %{buildroot}%{_bindir}/trace_cntl
%{__install} script/trace_delta %{buildroot}%{_bindir}/trace_delta
%{__install} script/bitN_to_mask %{buildroot}%{_bindir}/bitN_to_mask
%{__install} script/trace_envvars %{buildroot}%{_bindir}/trace_envvars

# Install manpages
%{__install} -d %{buildroot}%{_mandir}/man1
%{__install} doc/t*.1 %{buildroot}%{_mandir}/man1/
%{__install} -d %{buildroot}%{_mandir}/man1p
%{__install} doc/t*.1p %{buildroot}%{_mandir}/man1p/

# Install kmod docs
%{__install} -d %{buildroot}%{_defaultdocdir}/kmod-%{kmod_name}-%{version}/
%{__install} -m644 doc/users_guide.txt %{buildroot}%{_defaultdocdir}/kmod-%{kmod_name}-%{version}/
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
