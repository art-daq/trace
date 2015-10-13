#! /bin/sh
#   This file (cvs_change_desc.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jun 16, 2000. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: cvs_rel_notes.sh,v $
#   $Revision$
#   $Date$

set -u
USAGE="usage: `basename $0` [--update_REL_NOTES] <nextVersion>
   <nextVersion> shoule be in \"vX_Y\" format, i.e v3_3.
   If --update_REL_NOTES is specified, output will automatically be prepended
   to doc/RELEASE.NOTES; otherwise it goes to stdout"
opts_wo_args='update_REL_NOTES'
opts_w_args='" "'
while op=`expr "${1-}" : '-\(.*\)'`;do
    shift
    if xx=`expr "$op" : '-\(.*\)'`;then
        op=$xx
    else
        # support for multiple single letter options after one dash
        op=`echo $op | perl -e '$_=<>;$_=~s/(.)/$1 /g;print "$_"'`
    fi
    for opt in $op;do
        eval "case \$opt in
        \\?|h|help) echo \"\$USAGE\"; exit 0;;
        $opts_wo_args)
            eval opt_\$opt=1;;
        $opts_w_args)
            if [ $# = 0 ];then echo option $opt requires argument; exit 1; fi
            eval opt_\$opt=\"'\$1'\";shift ;;
        *)  echo \"invalid option: \$opt\";echo \"\$USAGE\" >&2; exit 1;;
        esac"
    done
done

if [ $# != 1 ];then echo "$USAGE" >&2;exit 1; fi

NEXT_VERSION=$1

get_latest_tag()
{
    # pick first cvs file
    cvs_files=`cat CVS/Entries | grep '^/' | sed -e 's|/||' -e 's|/.*||'`
    for file in $cvs_files;do
        echo "get_latest_tag trying file $file" 1>&2
        latest_tag=`cvs status -v $file | grep "(revision:" | head -1`
        if [ "$latest_tag" ];then
            expr "$latest_tag" : '[ 	]*\([^ 	]*\)'
            break
        fi
    done
}

BASEPOINT_TAG=`get_latest_tag`

cvs_rev_change_list=`\`dirname $0\`/cvs_list_changed_files.sh $BASEPOINT_TAG`

while true;do
    Cvs_Module=`cat CVS/Repository`
    if [ $Cvs_Module = `basename $Cvs_Module` ];then
        echo "CVS module is $Cvs_Module" >&2
        break
    fi
    if [ ! -f ../CVS/Repository ];then break;fi
    cd ..
done

if [ "${opt_update_REL_NOTES-}" ];then
    if [ ! -f doc/RELEASE.NOTES ];then
        echo ERROR: doc/RELEASE.NOTES not found
        exit 1
    fi
    tfile=/tmp/trace_cvs_rel_notes.$$
    exec >$tfile  # io redirection is permanent
fi
    

echo
echo "CHANGE(S) FOR VERSION $NEXT_VERSION"
echo
echo "  The number of files changed: "`echo "$cvs_rev_change_list" | wc -l`
echo
echo "  prvTagRev curTagRev     File"
echo "  --------- --------- --------------------------------------------"
echo "$cvs_rev_change_list" | sed -e 's/^/  /'
echo
echo

RET="
"
IFS_default=$IFS
IFS=$RET
for i in $cvs_rev_change_list;do
    revRange=`expr "$i" : '[ ]*\([^ ]*\)'` 
        file=`expr "$i" : '[ ]*[^ ]*[ ]*\([^ ]*\)'`
    cvs -q log -r$revRange $file
done | perl -e '
while (<>)
{   if    (/^Working file: (\S+)/) { $work_file=$1; }
    elsif (/^RCS file: (\S+)/)     { $rcs_file=$1; }
    elsif (/^symbolic names:/)
    {   $tag_rev="";
	while (! (($_=<>)=~/^\S/)) # chew through all symbolic names
	{   #print;
	    if (/'$BASEPOINT_TAG': (\S+)/)	# tricky shell quoting
	    {   $tag_rev=$1;
	        last;
	    }
	}
	while (!(($_=<>)=~/^revision (\S+)/)) { ; } # get to first description
	$first_rev=$1;
	if ($first_rev ne $tag_rev)
	{   print "  File: $work_file:\n";
	    $prev_line=$_;
	    while (!(($_=<>)=~/^revision $tag_rev/)&&!/^=========/)
	    {   if (  !($prev_line=~/^revision/)&&!($prev_line=~/^date:/)
                    &&!($prev_line=~/^----/)    &&!($prev_line=~/^branches:/))
                {   print "  $prev_line";
                }
	        $prev_line=$_;
	    }
	    if (/^=========/) { print "$prev_line"; } # added file
	    print "  ----------------------------------------------------\n";
	}
	#else
	#{   print "skip file $work_file because no changes since tag\n";
	#}
    }
}'
IFS="$IFS_default"

echo ==========================================================================
echo

if [ "${opt_update_REL_NOTES-}" ];then
    mv -f doc/RELEASE.NOTES doc/RELEASE.NOTES~
    cat $tfile doc/RELEASE.NOTES~ >doc/RELEASE.NOTES
    rm $tfile
    echo doc/RELEASE.NOTES has been updated >&2
fi
