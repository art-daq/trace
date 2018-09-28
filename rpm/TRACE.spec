Name:           trace
Version:        v3
Release:        1%{?dist}
Summary:        Logging tool that allows fast and/or slow logging, dyamically

License:        Fermilab
URL:            https://cdcvs.fnal.gov/redmine/projects/trace
Source0:        trace-v3.tar
BuildRequires:	gcc
BuildRequires:	make
Requires:	bash

# Reading articles on rikers.org/rpmbook/node69.html for assistance with this
# Reading article from university of michigan for assistance with header files



%description
Control tracing via environment variables and dynamically from outside program

%prep
%setup -q


%build
make OUT=$PWD all

%install
## Need to specify files to be installed in %files in order for this section to work. 
## See: https://www.cyberciti.biz/faq/rhel-centos-linuxrpmbuild-error-installed-but-unpackaged-files-found/
#############################################################################
## mkdir $RPM_BUILD_ROOT/usr/include/, give permissions, and cp trace.h   ###
#############################################################################
mkdir -p      $RPM_BUILD_ROOT/usr/include/TRACE/
chmod -R 755  $RPM_BUILD_ROOT/usr/include/TRACE/
chmod -R 755 ~/rpmbuild/BUILD/%{name}-%{version}/include/*
cp           ~/rpmbuild/BUILD/%{name}-%{version}/include/trace.h $RPM_BUILD_ROOT/usr/include/TRACE/trace.h
cp           ~/rpmbuild/BUILD/%{name}-%{version}/include/tracemf.h $RPM_BUILD_ROOT/usr/include/TRACE/tracemf.h

##############################################################################
## mkdir $RPM_BUILD_ROOT/usr/bin, give permissions, and cp trace_delta.pl  ###
##############################################################################
mkdir -p      $RPM_BUILD_ROOT/usr/bin/
chmod -R 755  $RPM_BUILD_ROOT/usr/bin/
chmod -R 755 ~/rpmbuild/BUILD/%{name}-%{version}/script/trace_delta.pl
cp           ~/rpmbuild/BUILD/%{name}-%{version}/script/trace_delta.pl $RPM_BUILD_ROOT/usr/bin/trace_delta.pl
#####################################################################################
## using same dir just created in last step, give permissions, and cp trace_cntl  ###
#####################################################################################
chmod -R 755 ~/rpmbuild/BUILD/%{name}-%{version}/Linux64bit+3.10-2.17/bin/trace_cntl
cp           ~/rpmbuild/BUILD/%{name}-%{version}/Linux64bit+3.10-2.17/bin/trace_cntl $RPM_BUILD_ROOT/usr/bin/trace_cntl
#########################################################################################
## mkdir $RPM_BUILD_ROOT/usr/etc/profile.d/, give permissions, cp trace.sh.functions  ###
#########################################################################################
mkdir -p      $RPM_BUILD_ROOT/etc/profile.d/
chmod -R 755  $RPM_BUILD_ROOT/etc/profile.d/
chmod -R 644  ~/rpmbuild/BUILD/%{name}-%{version}/script/trace.sh.functions
mv            ~/rpmbuild/BUILD/%{name}-%{version}/script/trace.sh.functions $RPM_BUILD_ROOT/etc/profile.d/trace.sh

########################################################################################
## mkdir -p $RPM_BUILD_ROOT/%{_mandir}/man1                                         ####
##                                                                                  ####
## this might help for making links within the spec:                                ####  
## https://stackoverflow.com/questions/7521980/packaging-symlinks-via-rpmbuild      ####
########################################################################################
mkdir -p      $RPM_BUILD_ROOT/%{_mandir}/man1
mkdir -p      $RPM_BUILD_ROOT/%{_mandir}/man1p
chmod -R 755  $RPM_BUILD_ROOT/%{_mandir}/man*
chmod -R 755  ~/rpmbuild/BUILD/%{name}-%{version}/doc/t*
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tlvls.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tlvls.1
#cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/trace.1 $RPM_BUILD_ROOT/%{_mandir}/man1/trace.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tlvlM.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlM.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tmode.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tmode.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/trace_cntl.1 $RPM_BUILD_ROOT/%{_mandir}/man1/trace_cntl.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tenv.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tenv.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tfreeze.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tfreeze.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tinfo.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tinfo.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/treset.1 $RPM_BUILD_ROOT/%{_mandir}/man1/treset.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tshow.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tshow.1
# cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/t.1 $RPM_BUILD_ROOT/%{_mandir}/man1/t.1
#### below this is what im working on

cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tonM.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tonM.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/toffM.1 $RPM_BUILD_ROOT/%{_mandir}/man1/toffM.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tlvlsSave.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlsSave.1
cp            ~/rpmbuild/BUILD/%{name}-%{version}/doc/tlvlsRestore.1 $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlsRestore.1


gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tonM.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/toffM.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlsSave.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlsRestore.1

## above this is what im working on

gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tfreeze.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tinfo.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/treset.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tshow.1
#gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/t.1
#gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/t.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tenv.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/trace_cntl.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tmode.1

gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlM.1
gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/tlvls.1
#gzip          $RPM_BUILD_ROOT/%{_mandir}/man1/trace.1
chmod 644     $RPM_BUILD_ROOT/%{_mandir}/man1/t*.1.gz
## add more man*


#ln -s          /usr/share/man/man1/t.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tinfo.1.gz

ln -s          /usr/share/man/man1/tshow.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tdelta.1.gz

#ln -s          /usr/share/man/man1/t.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/treset.1.gz
ln -s          /usr/share/man/man1/tonM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tonMg.1.gz
ln -s          /usr/share/man/man1/tonM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tonS.1.gz
ln -s          /usr/share/man/man1/tonM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tonT.1.gz
ln -s          /usr/share/man/man1/tonM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tonSg.1.gz
ln -s          /usr/share/man/man1/tonM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tonTg.1.gz
#ln -s          /usr/share/man/man1/t.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tonS.1.gz
#ln -s          /usr/share/man/man1/t.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/toffS.1.gz
#ln -s          /usr/share/man/man1/t.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/toffM.1.gz

ln -s          /usr/share/man/man1/tmode.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tmodeM.1.gz
ln -s          /usr/share/man/man1/tmode.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tmodeS.1.gz

ln -s          /usr/share/man/man1/trace_cntl.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tcntl.1.gz
#######
ln -s          /usr/share/man/man1/tlvls.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/ttids.1.gz
ln -s          /usr/share/man/man1/tlvlM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlS.1.gz
ln -s          /usr/share/man/man1/tlvlM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlT.1.gz
ln -s          /usr/share/man/man1/tlvlM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlMg.1.gz
ln -s          /usr/share/man/man1/tlvlM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlSg.1.gz
ln -s          /usr/share/man/man1/tlvlM.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tlvlTg.1.gz
#######
#ln -s          /usr/share/man/man1/tmode.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tmodeM.1.gz
#ln -s          /usr/share/man/man1/tmode.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/tmodeS.1.gz


%files
%doc    ~/rpmbuild/BUILD/trace-v3/README
%doc    ~/rpmbuild/BUILD/trace-v3/src_example/module/s*
%doc    ~/rpmbuild/BUILD/trace-v3/src_example/module1/other_module.c
%doc    ~/rpmbuild/BUILD/trace-v3/src_example/userspace/basic_c.c
%doc    ~/rpmbuild/BUILD/trace-v3/src_example/userspace/example*
%doc    ~/rpmbuild/BUILD/trace-v3/src_example/userspace/inactive_tlvls.c
%doc    ~/rpmbuild/BUILD/trace-v3/src_example/userspace/just*
%doc    ~/rpmbuild/BUILD/trace-v3/src_example/userspace/no_std.cc
#%doc    ~/rpmbuild/BUILD/trace-v3
         /%{_includedir}/TRACE/trace.h
         /%{_includedir}/TRACE/tracemf.h
         /%{_bindir}/trace_delta.pl
         /%{_bindir}/trace_cntl
         /etc/profile.d/trace.sh
### You will never need to change anything in this files section when adding new man pages as 
### long as the file is going to a /usr/share/man/man* subdirectory and starts with t:
         /%{_mandir}/man*/t*.*.gz





%changelog
* Mon Jul 30 2018 Matthew Adas <madas@fnal.gov> v3_13_09-1
* Wed Jun 13 2018 Matthew Adas <madas@fnal.gov> v3_13_09-1
- TRACE package

