#!/bin/sh
 # This file (trace_rpms_from_repo.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Jun 15, 2018. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#
rev="$Revision: 1014 $$Date: 2018-08-09 15:56:04 -0500 (Thu, 09 Aug 2018) $"
cd
echo work in progress
echo number of arguments given: $# \$0=$0
svn ls http://cdcvs.fnal.gov/subversion/trace-svn/tags | tail

rpmdev-setuptree
##### Trying to think about how to make a directory called tmp_rpm to build, package, and install trace with. Need to figure a way for TRACE.spec to access this in the same way it accessess rpmbuild and its subdirectories
## mkdir tmp_rpm && cd tmp_rpm
## rpmdev-setuptree will still create rpmbuild directory tree in home directory. Need to find a way to change this. 
#
###########################   rpmrc???? #######################

wget https://cdcvs.fnal.gov/redmine/projects/trace/repository/svn/raw/trunk/rpm/TRACE.spec -O ~/rpmbuild/SPECS/TRACE.spec

# now get/create the source tar file
svn export https://cdcvs.fnal.gov/subversion/trace-svn/trunk trace-v3 | tail
rm -r trace-v3/rpm && rm trace-v3/script/trace_rpms_from_repo.sh
tar -cf rpmbuild/SOURCES/trace-v3.tar trace-v3
rm -r trace-v3
rpmbuild -ba ~/rpmbuild/SPECS/TRACE.spec
echo Someday you rpms will be in ~/rpmbuild/RPMS:
ls ~/rpmbuild/RPMS && ls ~/rpmbuild/RPMS/x86_64 && printf "\n`ls ~/rpmbuild/RPMS | wc -l` `echo directory & ` `ls ~/rpmbuild/RPMS/x86_64 | wc -l` `echo rpms`\n\n"
echo "


##################################################################################################################################################################################################################################################################

There is currently a bug in this version of TRACE because the main repository hasn't synced to svn properly. Level M masks are affected.

#################################################################################################################################################################################################################################################################


install your trace rpm with root permission (i.e.: sudo)

once installation is complete, make commands available by either opening new terminal window or run command: \". /etc/profile.d/trace.sh\"

When compiling, use -I option to avoid errors in accessing trace.h and tracemf.h (i.e.: gcc -o basictrace -I/usr/include/TRACE/ /usr/share/doc/trace-v3/basic_c.c). Try example given and run ./basictrace. This note is also in the trace man pages."


