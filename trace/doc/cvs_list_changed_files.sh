#! /bin/sh
#   This file (cvs_rel_notes.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jun 16, 2000. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: cvs_list_changed_files.sh,v $
#   $Revision: 1.1 $
#   $Date: 2014-06-18 19:57:10 $

# get a list of the files which have changed since a specified tag

set -u

USAGE="`basename $0` <basepointTag>"

if [ $# != 1 ];then echo "$USAGE" >&2; exit 1; fi

BASEPOINT_TAG=$1

while true;do
    Cvs_Module=`cat CVS/Repository`
    if [ $Cvs_Module = `basename $Cvs_Module` ];then
        echo "CVS module is $Cvs_Module" 1>&2
        break
    fi
    if [ ! -f ../CVS/Repository ];then break;fi
    cd ..
done

Cvs_Module_re=`echo $Cvs_Module | sed 's|/|\\\\/|g'`

changed_files=`cvs -q status -v | perl -e '
while (<>)
{   if (/^File: (\S+)/)
    {   $file=$1;
        <>; $_=<>; $_ =~ /\s([0-9.]+)\s/;
        $work_rev=$1;
        $_=<>; $_ =~ /Repository revision:\s+\S+\s+\S+\/'$Cvs_Module_re'\/(\S+),v/;    # tricky shell quoting
        $repos_file=$1;
        #print "repos_file is $repos_file\n";
        while (<>)
        {   if (/Existing Tags:/)
            {   while (<>)
                {   if (/\s+'$BASEPOINT_TAG'+\s+\(revision: ([0-9.]+)/) # tricky shell quoting
                    {   $tag_rev=$1;
                        last;
                    }
                    elsif (/^===========/)      # if a file has been added
                    {   $tag_rev="";
                        last;
                    }
                }
                last;
            }
        }
        if ($work_rev ne $tag_rev)
        {   printf "%9s:%-9s %s\n", $tag_rev, $work_rev, $repos_file;
        }
    }
}'`

echo "$changed_files"
