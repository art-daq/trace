#! /bin/sh
 # This file (trace_gnuplot.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Dec  6, 2016. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: .emacs.gnu,v $
 # rev='$Revision: 1.30 $$Date: 2016/03/01 14:27:27 $'
USAGE="\
  usage: tshow|grep 're' | tdelta -d 1 -ct 1 | `basename $0`
"

set -u

if tty -s && test $# -eq 0;then
    echo "$USAGE"
    exit
fi

awk '
BEGIN{
 print"set xdata time"
 print"set timefmt \"%m-%d %H:%M:%S\""  # input data can be multi column time format
 print"set format x \"%H:%M:%S\""
 #print"set terminal png     # comment this and next line to just display on X"
 #print"set output \"t.png\""
 print"plot \"-\" using 1:6"
}
{
 print
}
END{
 print"e"
 system("sleep 10")
}
' | gnuplot

#display t.png
