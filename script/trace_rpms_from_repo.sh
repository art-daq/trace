#!/bin/sh
 # This file (trace_rpms_from_repo.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Jun 15, 2018. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#
rev="$Revision: 1050 $$Date: 2019-02-19 16:23:54 -0600 (Tue, 19 Feb 2019) $"
USAGE="\
  usage: `basename $0` [opts] [revtag]
example: `basename $0`   # rpms from latest revtag
NOTE: works for revisions v3_13_12 and later.
opts: -l to list last 10 revs.
"
VUSAGE=""

eval env_opts=\${`basename $0 | sed 's/\.sh$//' | tr 'a-z-' 'A-Z_'`_OPTS-} # can be args too
eval "set -- $env_opts \"\$@\""
# Process script arguments and options
op1chr='rest=`expr "$op" : "[^-]\(.*\)"`; test -n "$rest" && set -- "-$rest" "$@"'
op1arg='rest=`expr "$op" : "[^-]\(.*\)"`; test -n "$rest" && set -- "$rest"  "$@"'
reqarg="$op1arg;"'test -z "${1+1}" &&echo opt -$op requires arg. &&echo "$USAGE" &&exit'
args= do_help= opt_v=0
while [ -n "${1-}" ];do
    if expr "x${1-}" : 'x-' >/dev/null;then
        op=`expr "x$1" : 'x-\(.*\)'`; shift   # done with $1
        leq=`expr "x$op" : 'x-[^=]*\(=\)'` lev=`expr "x$op" : 'x-[^=]*=\(.*\)'`
        test -n "$leq"&&eval "set -- \"\$lev\" \"\$@\""&&op=`expr "x$op" : 'x\([^=]*\)'`
        case "$op" in
        \?*|h*)     eval $op1chr; do_help=1;;
        v*)         eval $op1chr; opt_v=`expr $opt_v + 1`;;
        x*)         eval $op1chr; test $opt_v -ge 1 && set -xv || set -x;;
        l*)         svn ls http://cdcvs.fnal.gov/subversion/trace-svn/tags | tail | tr -d /; exit;;
        -use-local) opt_use_local=1;;
        *)          echo unknown option -$op$leq$lev; do_help=1;; # see dash_opts5.sh for mix {,un}known
        esac
    else # allow mix of opts and arg (i.e. opts after args)
        aa=`echo "$1" | sed -e "s/'/'\"'\"'/g"` args="$args '$aa'"; shift
    fi
done
eval "set -- $args \"\$@\""; unset args aa
test $# -gt 1 && echo 'Too many arguments. $#='$# && do_help=1
test -n "${do_help-}" && { echo "$USAGE";test $opt_v -ge 1 && echo "$VUSAGE"; exit; }
set -u

test $# -eq 1 \
    && version=$1 \
    || version=`svn ls http://cdcvs.fnal.gov/subversion/trace-svn/tags | tail -1 |tr -d /`

case $version in
v*)         revtag=tags/$version ;;
trunk|dev*) revtag=trunk         version=v0;;
esac

this_script_dir=`dirname $0`; this_script_dir=`cd $this_script_dir >/dev/null 2>&1;pwd`

cd  # change to home dir just for good measure
# from rpmdevtools rpm
rpmdev-setuptree
## rpmdev-setuptree will create rpmbuild directory tree in home directory.
cd rpmbuild

if [ -n "${opt_use_local-}" -a -f $this_script_dir/../rpm/TRACE.spec ];then
    cp $this_script_dir/../rpm/TRACE.spec SPECS/TRACE.spec
else
    wget https://cdcvs.fnal.gov/redmine/projects/trace/repository/svn/raw/$revtag/rpm/TRACE.spec \
         -O SPECS/TRACE.spec
fi

wget https://cdcvs.fnal.gov/redmine/projects/trace/repository/svn/raw/$revtag/rpm/kmodtool-TRACE.sh \
     -O SOURCES/kmodtool-TRACE.sh

# adjust Version: in spec
sed -i "s/Version:.*/Version: $version/" SPECS/TRACE.spec

# now get/create the source tar file
cd SOURCES
svn export https://cdcvs.fnal.gov/subversion/trace-svn/$revtag TRACE-$version | tail
tar cjf TRACE-$version.tar.bz2 TRACE-$version
rm -fr TRACE-$version
cd ../SPECS

echo Start rpmbuild -ba TRACE.spec
rpmbuild -ba TRACE.spec


echo Someday your rpms will be in ~/rpmbuild/RPMS:
ls ~/rpmbuild/RPMS && ls ~/rpmbuild/RPMS/x86_64 && printf "\n`ls ~/rpmbuild/RPMS | wc -l` `echo directory & ` `ls ~/rpmbuild/RPMS/x86_64 | wc -l` `echo rpms`\n\n"

echo "
install your trace rpm with root permission (i.e.: sudo)

once installation is complete, make commands available by either opening new terminal window or run command: \". /etc/profile.d/trace.sh\"

When compiling, use -I option to avoid errors in accessing trace.h and tracemf.h (i.e.: gcc -o basictrace -I/usr/include/TRACE/ /usr/share/doc/trace-v3/basic_c.c). Try example given and run ./basictrace. This note is also in the trace man pages.
"


