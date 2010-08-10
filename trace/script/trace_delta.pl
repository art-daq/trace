#!/usr/bin/env perl
#   This file (trace_delta.pl) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jan  6, 2000. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: trace_delta.pl,v $
$version = '$Revision: 1.36 $';
#   $Date: 2010-08-10 19:28:17 $

use Time::Local; # timelocal()

$USAGE = "\
$version
usage: $0 [options] [files]...

options:
-cpu  <col_spec>     column for cpu field.
-c    <cols_spec>    columns to include in output
-d    <cols_spec>    columns for non cpu specific delta
-dc   <cols_spec>    columns for cpu specific delta
-dw   <width>        delta column width
-pre  <re>           allow processing if true i.e: '/(begin|end)/ && !/916951/'
-post <re>           post processing filter (of output lines)
-b                   output delta before associated column
-stats               min, max, ave stats on delta (except cpu deltas)
-i                   invert (use this is deltas are always negative)
-r                   change \"timeStamp\" column to \"relative\" (currently
                     only works when header line is included)
-ct   <col_spec>     make time more readable (convert time)
-syscall             attempt read of /usr/include/asm/unistd.h to convert
                     system call numbers in string \"system call \\d+\$\" to names
-show_sub            development/debug option

cols_spec examples:  (Note: cols are zero indexed)
   CPU,message
   3..5,0,11,Rest

Note: If -c is used, columns specified in the -d/-dc options must be
      a subset of the options specified.

defaults:
-d  timeStamp        Note: not -dc. Use -dc timeStamp if desired
-dw 10
";
#
#   RECORD OPTIONS
#
# define options - ORDER IS IMPORTANT (see below). 2nd field == 1 if arg.
@opts=( 'cpu,1','dw,1','dc,1','d,1','ct,1','c,1','pre,1','post,1','b,0','v,0'
       ,'stats,0','i,0','r,0','show_sub,0','"syscall",0','xxx,0');  # syscall needs to be quoted as it is a function
while ($ARGV[0] =~ /^-/)
{   $_ = shift;
    if (/^-[h?]/) { die "$USAGE\n"; }
    $found = 0;
    foreach $optset (@opts)
    {   eval "\@opt = ($optset)";
	#print STDERR "trying opt $opt[0] from ($optset)\n";
	if (/^-$opt[0]/)  # NOTE: currently NOT /^-$opt[0]$/, hence order important (ref. above)
	{   if ($opt[1]) { eval "\$opt_$opt[0] = shift"; }
	    else         { eval "\$opt_$opt[0] = 1"; }
	    $found = 1; last;
	}
    }
    if (!$found)
    {   die "Unrecognized option: $_\n$USAGE\n";
    }
}

#
#   GET FIRST LINE .. it is a basis for alot of processing
#   it should either a line with headings that are right justified in the
#   colums OR a data line.
#
$line = <>;

#
#   PROCESS OPTIONS
#
if ("$opt_dw" ne "") { $delta_width = $opt_dw; } else { $delta_width = 10; }

#
# example: col_spec_to_re( cpu ) will return something like "........(.....)"
#
sub col_spec_to_re
{   $spec = shift @_;
    eval "\$col_spec = \$${spec}_col_spec";  # kludgie way to pass argument
                                             # i.e. use col_spec_to_re( tmp )
                                             # to pass tmp_col_spec
                                             # and return re in tmp_re
    #print STDERR "col_spec_to_re(spec=$spec)\n";
    #print STDERR "col_spec=$col_spec\n";
    if ($col_spec =~ /^\d+$/)
    {   if ($col_spec == 0)
	{   $re = '(\s*\S+)';
	    $line =~ /$re/; $data = $1;
	    $re = '(' . '.' x length($data) . ')';
	}
	else
	{   $re = '(' . '\s*\S+' . '\s+\S+' x ( $col_spec - 1 ) . ')(\s+\S+)';
	    $line =~ /$re/; $leader = $1; $data = $2;
	    $re = '.' x length($leader) . '(' . '.' x length($data) . ')';
	}
    }
    else
    {   $re = '^((\s+\S+)*)(\s+' . $col_spec . ')';
	$line =~ /$re/; $leader = $1; $data = $3;
	$re = '.' x length($leader) . '(' . '.' x length($data) . ')';
    }
    if (!"$data")
    {   die "problem finding column associated with \"$col_spec\" in first line\n";
    }
    eval "\$${spec}_re = \$re";
}

if ("$opt_ct" ne "")
{   use POSIX;			# for strftime  NOTE: this is "used" at compile
    $ct_col_spec = $opt_ct;     # time, i.e. even if -ct is not specified.
    &col_spec_to_re( ct );
}

$delta_idx=0;

sub get_cpu_re
{   if ($cpu_col_spec =~ /\d+/)
    {   # special - allows for more delta tricks
	if ($cpu_col_spec == 0)
	{   $cpu_re = '(\s+\S+)';
	    $line =~ /$cpu_re/; $data = $1;
	}
	else
	{   $cpu_re = '(' . '\s+\S+' x $col_spec . ')(\s+\S+)';
	    $line =~ /$cpu_re/; $leader = $1; $data = $2;
	}
    }
    else { &col_spec_to_re( cpu ); } # refine cpu_re
}

#print STDERR "before if (opt_dc) opt_dc=$opt_dc\n";
if ("$opt_dc" ne "")      # if "columns for *cpu specific* deltas" where specified
{   # we need a cpu spec
    if ("$opt_cpu" ne "") { $cpu_col_spec = $opt_cpu;}
    else                  { $cpu_col_spec = CPU;}
    &get_cpu_re;

    eval "\@opt_eval = ($opt_dc)";
    foreach $col (@opt_eval)
    {   $tmp_col_spec = $col;
	&col_spec_to_re( tmp );
	$delta_cntl[$delta_idx]{re}   = $tmp_re;
	$delta_cntl[$delta_idx]{cpu}  = 1;
	print STDERR "col=$col ";
	if ($col eq timeStamp && $opt_r)
	{   $delta_cntl[$delta_idx]{rel} = 1;
	    print STDERR " setting relative time\n";
	}
	print STDERR "\n";
	$default_col[$delta_idx] = $tmp_re;
	$delta_idx++;
    }
}
#print STDERR "before if (opt_d) opt_d=$opt_d\n";
if ("$opt_d" ne "")      # if "columns for *cpu specific* deltas" where specified
{   eval "\@opt_eval = ($opt_d)";
    #print STDERR "in if (opt_d) opt_d=$opt_d\n";
    foreach $col (@opt_eval)
    {   $tmp_col_spec = $col;
	#print STDERR "calling col_spec_to_re( tmp )\n";
	&col_spec_to_re( tmp );
	$delta_cntl[$delta_idx]{re}   = $tmp_re;
	if ($col eq timeStamp && $opt_r)
	{   $delta_cntl[$delta_idx]{rel} = 1;
	}
	$default_col[$delta_idx] = $tmp_re;
	$delta_idx++;
    }
}

if ("$opt_d" eq "" && "$opt_dc" eq "")  # need to try default
{   $tmp_col_spec = timeStamp;
    &col_spec_to_re( tmp );
    $delta_cntl[$delta_idx]{re}   = $tmp_re;
    if ($opt_r)
    {   $delta_cntl[$delta_idx]{rel} = 1;
    }
    $default_col[$delta_idx] = $tmp_re;
}

sub find_delta
{   $re = shift @_;
    local($idx) = 0;
    foreach $idx (0..$#delta_cntl)
    {   if ($re eq $delta_cntl[$idx]{re}) { return $idx; }
    }
    return -1;
}

sub get_delta_data
{   $delta_d = shift @_;
    if ($delta_d =~ /^[\d.]+$/o)
    {   $ret = $delta_d;
    }
    if ($delta_d =~ /^([0-9]+):([0-9]+):([0-9][0-9])\.(\d\d\d\d\d\d)$/o)
    {   # convert to integer microseconds
        $year=1970; $month=0; $date=1; $hour=$1; $minute=$2; $seconds=$3;
        $time_then = timelocal(($seconds,$minute,$hour,$date,$month,$year,0,0,0));
	$ret = $time_then . $4;
    }
    else
    {   $ret = $delta_d;
    }
    #print STDERR "ret=$ret\n";
    return $ret;
}

$cntl_idx = 0;
if ("$opt_c" ne "")      # if the -c option to specify which columns are in output
{   eval "\@opt_eval = ($opt_c)";
    foreach $col (@opt_eval)
    {   if    ($col =~ /^R(est)*$/)
	{   $prev = $col_cntl[$cntl_idx-1]{re};
	    $prev =~ s/[()]//g;
	    $col_cntl[$cntl_idx]{re} = "$prev(.*)";
	}
	elsif ($col =~ /^message$/)
	{   $tmp_col_spec = $col;
	    &col_spec_to_re( tmp );
	    $tmp_re =~ /^(\.*)/; $leader = $1;
	    $col_cntl[$cntl_idx]{re} = "$leader(.*)";
	}
	else
	{   $tmp_col_spec = $col;
	    &col_spec_to_re( tmp );
	    $col_cntl[$cntl_idx]{re} = $tmp_re;
	    $delta_idx = &find_delta( $col_cntl[$cntl_idx]{re} );
	    if ($delta_idx != -1)
	    {   $col_cntl[$cntl_idx]{delta} = 1;
		if ($delta_cntl[$delta_idx]{cpu})
		{   $col_cntl[$cntl_idx]{delta_cpu} = 1;
		}
		if ($delta_cntl[$delta_idx]{rel})
		{   $col_cntl[$cntl_idx]{rel} = 1;
		}
	    }
	    elsif ($col eq timeStamp && $opt_r)
	    {   $col_cntl[$cntl_idx]{rel} = 1;
	    }
	    if ("$opt_ct" ne "" && $col_cntl[$cntl_idx]{re} eq $ct_re)
	    {   $col_cntl[$cntl_idx]{ct} = 1;
	    }
	}
	$cntl_idx++;
    }
}
else
{   # build output_cntl that outputs all input columns in order

    # sort -d and -dc
    sort @default_col;

    $prev="";
    foreach $idx (0..$#delta_cntl)
    {   $default_col[$idx] =~ /^(\.*)/; $leader = $1;
	$prev =~ s/[()]//g;
	if ("$prev") { $leader =~ s/$prev//; } # subtract prev from leader
	if ("$leader")
	{    $col_cntl[$cntl_idx]{re} = "$prev($leader)";
	     $cntl_idx++;
	}
	$col_cntl[$cntl_idx]{re} = $default_col[$idx];
	$delta_idx = &find_delta( $col_cntl[$cntl_idx]{re} );
	$col_cntl[$cntl_idx]{delta} = 1;
	if ($delta_cntl[$delta_idx]{cpu})
	{   $col_cntl[$cntl_idx]{delta_cpu} = 1;
	}
	if ($delta_cntl[$delta_idx]{rel})
	{   $col_cntl[$cntl_idx]{rel} = 1;
	}
	if ("$opt_ct" ne "" && $col_cntl[$cntl_idx]{re} eq $ct_re)
	{   $col_cntl[$cntl_idx]{ct} = 1;
	}
	$prev = $col_cntl[$cntl_idx]{re};
	$cntl_idx++;
    }
    $prev =~ s/[()]//g;
    $col_cntl[$cntl_idx]{re} = "$prev(.*)";
}

#
#   BUILD sub process_line FOR SPEED
#
$sub = "
    sub process_line
    {   ";
if ("$opt_pre" ne "")
{   $sub .= "
        return unless ($opt_pre);";
}
$sub .= "
        \$out_line = \"\";";
if ("$opt_syscall" ne "" && -f "/usr/include/asm/unistd.h")
{   open( SYSCALLFILE, "</usr/include/asm/unistd.h" );
    #print STDERR "opened /usr/include/asm/unistd.h for syscall translation\n";
    while (<SYSCALLFILE>)
    {    if (/define\s+__NR_(\S+)\s+(\d+)/) { $syscal[$2] = $1; } #print STDOUT "$syscal[$2] = $1\n"; }
    }
    $sub .= "
        \$line =~ s/ system call (\\d+)\$/ system call \$syscal[\$1](\$1)/;";
}

for $idx (0..$#col_cntl)
{   $sub .= "

        # processing for col_cnt[$idx]
        if (!(\$line=~/$col_cntl[$idx]{re}/o))  # must check for, i.e. blank line
        {   chop(\$line);
            #\$out_line = \$line;";
    $rest_re=$col_cntl[$idx]{re};
    $rest_re=~s/\(.*//g;
    $sub .= "
             \$line=~/$rest_re(.*)/;
             \$out_line .= \$1;";
            

    if ("$opt_post" ne "")
    {   $sub .= "
            \$_ = \$out_line;
            return unless ($opt_post); # re operates on \$_";
    }
    $sub .= "
            print STDOUT \"\$out_line\\n\";
            return;
        }
        else
        {   \$data = \$1;";
    if ($col_cntl[$idx]{rel})
    {   $sub .= "
            if (!\$col_cntl[$idx]{rel_start_val})
            {   \$col_cntl[$idx]{rel_start_val} = \$data;
            }
            \$data = sprintf( \"%*s\", length(\$data), \$data - \$col_cntl[$idx]{rel_start_val} );"
    }
    if (!$opt_b)
    {   
	if ($col_cntl[$idx]{ct}) # if converting time
	{   $sub .= "
            if (\$data =~ /^\\s*(\\d+)(\\d\\d\\d\\d\\d\\d)\$/o)
            {   \$seconds = \$1;
                \$useconds = \$2;
                \$str = strftime( \"%a %H:%M:%S.\$useconds\", localtime(\$seconds) );
                \$out_line .= sprintf( \"%*s\", length(\$data), \$str );
	    }
            else
            {   \$out_line .= \$data; # delta will come after
            }";
	}
	else
	{   $sub .= "
            \$out_line .= \$data; # delta will come after";
	}
    }
    if ($col_cntl[$idx]{delta})
    {   $sub .= "
            \$delta_data = &get_delta_data(\$data);
            if (\$delta_data =~ /^\\s*[-]*[\\d.]+/o)
            {   #print STDERR \"delta_data is \$delta_data\\n\";";
	if ($col_cntl[$idx]{delta_cpu})
	{   $sub .= "
                \$line =~ /$cpu_re/o;
                \$cpu = \$1;";
	}
	else
	{   $sub .= "
                \$cpu = 0;  # something consistant for non-cpu specific delta";
	}
	if ("$opt_i") # invert sense of delta
	{   $sub .= "
                if (!\$col_cntl[$idx]{\"prev\$cpu\"})
                {   \$col_cntl[$idx]{\"prev\$cpu\"} = \$delta_data;
                    \$delta = sprintf( \"%*d\", $delta_width, \$delta_data-\$col_cntl[$idx]{\"prev\$cpu\"} );";
        }
	else
	{   $sub .= "
                if (!\$col_cntl[$idx]{\"prev\$cpu\"})
                {   \$col_cntl[$idx]{\"prev\$cpu\"} = \$delta_data;
                    \$delta = sprintf( \"%*d\", $delta_width, \$col_cntl[$idx]{\"prev\$cpu\"}-\$delta_data );";
        }
	if ("$opt_stats")
	{    $sub .= "
                    \$col_cntl[$idx]{stat}  = init;
                    \$template=\$line;";
	}
	if ("$opt_i") # invert sense of delta
	{   $sub .= "
                }
                else
                {
                    \$delta = sprintf( \"%*s\", $delta_width, \$delta_data-\$col_cntl[$idx]{\"prev\$cpu\"} );
                    \$col_cntl[$idx]{\"prev\$cpu\"} = \$delta_data;";
	}
	else
	{   $sub .= "
                }
                else
                {
                    \$delta = sprintf( \"%*s\", $delta_width, \$col_cntl[$idx]{\"prev\$cpu\"}-\$delta_data );
                    \$col_cntl[$idx]{\"prev\$cpu\"} = \$delta_data;";
	}
	if ("$opt_stats")
	{    $sub .= "
                    \$col_cntl[$idx]{stat}  = \$delta;";
	}
	$sub .= "
                }
            }
            elsif (\$data =~ /^-+\$/o)
            {   \$delta = sprintf( \"%s\", '-' x $delta_width );";
	if ("$opt_stats")
	{    $sub .= "
                \$col_cntl[$idx]{stat}  = skip;";
	}
	$sub .= "
            }
            else
            {   \$delta = sprintf( \"%*s\", $delta_width, 'delta' );";
	if ("$opt_stats")
	{    $sub .= "
                \$col_cntl[$idx]{stat}  = skip;";
	}
	$sub .= "
            }
            \$out_line .= \$delta;
            #print STDERR \"delta=\$delta out_line=\$out_line\\n\";";
    };
    if ($opt_b)
    {
	if ($col_cntl[$idx]{ct}) # if converting time
	{   $sub .= "
            if (\$data =~ /^\\s*(\\d+)(\\d\\d\\d\\d\\d\\d)\$/o)
            {   \$seconds = \$1;
                \$useconds = \$2;
                \$str = strftime( \"%a %H:%M:%S.\$useconds\", localtime(\$seconds) );
                \$out_line .= sprintf( \"%*s\", length(\$data), \$str );
	    }
            else
            {   \$out_line .= \$data; # delta was before
            }";
	}
	else
	{   $sub .= "
            \$out_line .= \$data; # delta was before";
	}
    }
    $sub .= "
        }";
}
if ("$opt_post" ne "")
{   $sub .= "
        \$_ = \$out_line;
        return unless ($opt_post); # re operates on \$_";
}
if ("$opt_stats")
{   # whiz thru and calc stats
    for $idx (0..$#col_cntl)
    {   next if (!$col_cntl[$idx]{delta});
	if ($col_cntl[$idx]{delta_cpu})
	{   $sub .= "
        \$line =~ /$cpu_re/o;
        \$cpu = \$1;";
	}
	else
	{   $sub .= "
        \$cpu = 0;  # something consistant for non-cpu specific delta";
	}
	$sub .= "
        if (!grep(/^\$cpu\$/,\@stat_cpus)) { push( \@stat_cpus,\$cpu ); }
        if (\$col_cntl[$idx]{stat} eq init)
        {   \$col_cntl[$idx]{\"min\$cpu\"} = 999999999; # arbitrarily large number as 1st 0 does not count
            \$col_cntl[$idx]{\"max\$cpu\"} = 0;
            \$col_cntl[$idx]{\"tot\$cpu\"} = 0;         # just init
        }
        elsif (\$col_cntl[$idx]{stat} ne skip)
        {   if    (\$col_cntl[$idx]{\"min\$cpu\"} > \$col_cntl[$idx]{stat} \
                   || \$col_cntl[$idx]{\"min\$cpu\"} eq \"\")
            {      \$col_cntl[$idx]{\"min\$cpu\"} = \$col_cntl[$idx]{stat};
            }
            elsif (\$col_cntl[$idx]{\"max\$cpu\"} < \$col_cntl[$idx]{stat})
            {      # printf STDOUT \"max new=\$col_cntl[$idx]{stat} old=%d\\n\",\$col_cntl[$idx]{\"max\$cpu\"};
                   \$col_cntl[$idx]{\"max\$cpu\"} = \$col_cntl[$idx]{stat};
            }
            \$col_cntl[$idx]{\"tot\$cpu\"} += \$col_cntl[$idx]{stat};
            \$col_cntl[$idx]{\"cnt\$cpu\"}++;
            #print STDERR \"processed stats for \$col_cntl[$idx]{stat}\\n\";
        }";
    }
}
$sub .= "
        print STDOUT \"\$out_line\\n\";
    }";

if ("$opt_show_sub") { print STDOUT "$sub\n"; }
eval $sub;

#
#   NOW DO THE REAL WORK
#
$_ = $line; # needed for opt_pre processing
&process_line;

while (<>)
{   $line = $_;
    &process_line;
}

if ("$opt_stats")
{
    # use last line as a template
    #print STDOUT "processing stats\n";
    if ("$template" ne "")
    {   #print STDOUT "using template=$template\n";
	$line = $template;
    }
    # else we default to using the template being the last line read in.
    for $cpu (@stat_cpus)
    {   print STDOUT "cpu=\"$cpu\"\n";
	foreach $ss (min,max,tot,ave,cnt)
	{   $out_line = "";
	    for $idx (0..$#col_cntl)
	    {
		if (!($line=~/$col_cntl[$idx]{re}/))
		{   $out_line = "problem with stats";
		    last;
		}
		else
		{   $data = $1;  # this will be the column of data (from the
		                 # last line, as noted above)
		    if (!$opt_b)
		    {   # stats and deltas do NOT come before (they are after)
			# so if this column has a delta, the heading comes
			# before, IF it is a column for which there is a delta.
			if ($col_cntl[$idx]{delta})
			{   $ssdata =sprintf( "%*s", length($data), $ss );
			}
			else
			{   $ssdata = $data;
			    $ssdata =~ s/./ /g; # use the data to get the
			                        # correct number of spaces.
			}
			$out_line .= $ssdata; # delta will come after
		    }
		    if ($col_cntl[$idx]{delta})  # if this is a column for
		    {                            # which 'delta' was specified
			#print STDOUT "\nin stats delta col, data=$data\n";
			if ($data =~ /^\s*[-]{0,1}\d+/o)
			{   if ($col_cntl[$idx]{"cnt$cpu"})
			    {   if    ($ss eq ave) { $col_cntl[$idx]{"${ss}$cpu"} = $col_cntl[$idx]{"tot$cpu"} / $col_cntl[$idx]{"cnt$cpu"}; }
				else               { $col_cntl[$idx]{"${ss}$cpu"} =~ s/^ *//; } # strip leading spaces
                                # since we are using strings (to get arbitrarily large i.e. 64bit, numbers,
                                # we should implement the "out of range" functionality...
				$max_outchars = $delta_width-1;
				if (   $col_cntl[$idx]{"${ss}$cpu"} =~ /^\d{1,$max_outchars}\./
				    || length($col_cntl[$idx]{"${ss}$cpu"}) <= $max_outchars )
				{   $delta = sprintf( "%*.*s", $delta_width, $max_outchars, $col_cntl[$idx]{"${ss}$cpu"} );
				}
				else
				{   $delta = sprintf( "%*.*s", $delta_width, $max_outchars, '*' x $max_outchars );
				}
			    }
			    else { $delta = sprintf( "%*s", $delta_width, "nocpu" ); }
			}
			elsif ($data =~ /^-+$/o)
			{   $delta = sprintf( "%s", '-' x $delta_width );
			}
			else
			{   $delta = sprintf( "%*s", $delta_width, 'delta' );
			}
			$out_line .= $delta;
		    };
		    if ($opt_b)  # do the labeling after
		    {
			if ($col_cntl[$idx]{delta})
			{   $data =sprintf( "%-*s", length($data), " $ss" );
			}
			else { $data =~ s/./ /g; }
			$out_line .= $data; # delta was before
		    }
		}
	    }
	    $out_line =~ s/\s*$//; # remove trailing white space (this is just
	                           # a little nicer when just looking at stats)
	    print STDOUT "$out_line\n";
	}
    }
}
