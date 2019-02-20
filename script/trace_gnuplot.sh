#! /bin/sh
 # This file (trace_gnuplot.sh) was created by Ron Rechenmacher <ron@fnal.gov> on
 # Dec  6, 2016. "TERMS AND CONDITIONS" governing this file are in the README
 # or COPYING file. If you do not have such a file, one can be obtained by
 # contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 # $RCSfile: .emacs.gnu,v $
 # rev='$Revision: 1.30 $$Date: 2016/03/01 14:27:27 $'
USAGE="\
   usage: tshow|grep 're' | tdelta -d 1 -ct 1 | `basename $0`
examples:
tshow | grep ' 30438 .*instantaneous' | tdelta -ct 1\\
 | awk '{print\$2,\$3,\$21}' | head -50000\\
 | \$TRACE_DIR/script/trace_gnuplot.sh --title=

tshow | grep ' 6114 .* d09 .*recv returned' | tdelta -d 1 -ct 1\\
 | awk '{print\$2,\$3,\$4}' | head -50000\\
 | \$TRACE_DIR/script/trace_gnuplot.sh --title=
"









set -u

if tty -s && test $# -eq 0;then
    echo "$USAGE"
    exit
fi

# In gnuplot, when timefmt is 2 columns, the "plot using" is given
# the first column of the date. For example, if data is:
#11-11 13:47:09.114305 203.364
# the "plot using" would be "plot using 1:3"
awk '
BEGIN{
# print"set xdata time"
# print"set timefmt \"%m-%d %H:%M:%S\""  # input data can be multi column time format
# print"set format x \"%H:%M:%S\""
# #print"set terminal png     # comment this and next line to just display on X"
 #print"set output \"t.png\""
# print"plot \"-\" using 1:3"
  print"binwidth=10000"
  print"bin(x,width)=width*floor(x/width)"
  print"plot \"-\" using (bin($1,binwidth)):(1.0) smooth freq with boxes"
#  print"plot \"-\" using (bin($1,binwidth)):(1.0) freq with boxes"
}
{print}
END{
 print"e"
 system("sleep 1000000")
}
' | gnuplot
#' | head -22

#display t.png
