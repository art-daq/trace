#!/usr/bin/perl
#   This file (trace_delta.pl) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jan  6, 2000. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: trace_delta.pl,v $
$version = '$Revision: 1.18 $';
#   $Date: 2000-01-12 23:17:49 $


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
# define options - order is important.
@opts=('cpu,1','dw,1','dc,1','d,1','c,1','pre,1','post,1','b,0','v,0','stats,0','show_sub,0');
while ($ARGV[0] =~ /^-/)
{   $_ = shift;
    if (/^-[h?]/) { die "$USAGE\n"; }
    $found = 0;
    foreach $optset (@opts)
    {   eval "\@opt = ($optset)";
	if (/^-$opt[0]/)
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
#
$line = <>;

#
#   PROCESS OPTIONS
#
if ("$opt_dw")   { $delta_width = $opt_dw; }   else { $delta_width = 10; }

sub col_spec_to_re
{   $spec = shift @_;
    eval "\$col_spec = \$${spec}_col_spec";

    if ($col_spec =~ /^\d+$/)
    {   if ($col_spec == 0)
	{   $re = '(\s+\S+)';
	    $line =~ /$re/; $data = $1;
	    $re = '(' . '.' x length($data) . ')';
	}
	else
	{   $re = '(' . '\s+\S+' x $col_spec . ')(\s+\S+)';
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

if ("$opt_dc")
{   # we need a cpu spec
    if ("$opt_cpu") { $cpu_col_spec = $opt_cpu;}
    else            { $cpu_col_spec = CPU;}
    &get_cpu_re;

    eval "\@opt_eval = ($opt_dc)";
    foreach $col (@opt_eval)
    {   $tmp_col_spec = $col;
	&col_spec_to_re( tmp );
	$delta_cntl[$delta_idx]{re}   = $tmp_re;
	$delta_cntl[$delta_idx]{cpu}  = 1;
	$default_col[$delta_idx] = $tmp_re;
	$delta_idx++;
    }
}
if ("$opt_d")
{   eval "\@opt_eval = ($opt_d)";
    foreach $col (@opt_eval)
    {   $tmp_col_spec = $col;
	&col_spec_to_re( tmp );
	$delta_cntl[$delta_idx]{re}   = $tmp_re;
	$default_col[$delta_idx] = $tmp_re;
	$delta_idx++;
    }
}

if (!"$opt_d" && !"$opt_dc")  # need to try default
{   $tmp_col_spec = timeStamp;
    &col_spec_to_re( tmp );
    $delta_cntl[$delta_idx]{re}   = $tmp_re;
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

$cntl_idx = 0;
if ("$opt_c")
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
if ("$opt_pre")
{   $sub .= "
        return unless ($opt_pre);";
}
$sub .= "
        \$out_line = \"\";";
for $idx (0..$#col_cntl)
{   $sub .= "

        # processing for col_cnt[$idx]
        if (!(\$line=~/$col_cntl[$idx]{re}/o))
        {   chop(\$line); \$out_line = \$line;";
    if ("$opt_post")
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
    if (!$opt_b)
    {   $sub .= "
            \$out_line .= \$data; # delta will come after";
    }
    if ($col_cntl[$idx]{delta})
    {   $sub .= "
            if (\$data =~ /^\\s*\\d+/o)
            {";
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
                if (!\$col_cntl[$idx]{\"prev\$cpu\"})
                {   \$col_cntl[$idx]{\"prev\$cpu\"} = \$data;
                    \$delta = sprintf( \"%*d\", $delta_width, \$col_cntl[$idx]{\"prev\$cpu\"}-\$data );";
	if ("$opt_stats")
	{    $sub .= "
                    \$col_cntl[$idx]{stat}  = init;";
	}
	$sub .= "
                }
                else
                {
                    \$delta = sprintf( \"%*d\", $delta_width, \$col_cntl[$idx]{\"prev\$cpu\"}-\$data );
                    \$col_cntl[$idx]{\"prev\$cpu\"} = \$data;";
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
            \$out_line .= \$delta;";
    };
    if ($opt_b)
    {   $sub .= "
            \$out_line .= \$data; # delta was before";
    }
    $sub .= "
        }";
}
if ("$opt_post")
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
            \$col_cntl[$idx]{\"ave\$cpu\"} = 0;         # just init
        }
        elsif (\$col_cntl[$idx]{stat} ne skip)
        {   if    (\$col_cntl[$idx]{\"min\$cpu\"} > \$col_cntl[$idx]{stat})
            {      \$col_cntl[$idx]{\"min\$cpu\"} = \$col_cntl[$idx]{stat};
            }
            elsif (\$col_cntl[$idx]{\"max\$cpu\"} < \$col_cntl[$idx]{stat})
            {      \$col_cntl[$idx]{\"max\$cpu\"} = \$col_cntl[$idx]{stat};
            }
            \$col_cntl[$idx]{\"ave\$cpu\"} += \$col_cntl[$idx]{stat};
            \$col_cntl[$idx]{\"cnt\$cpu\"}++;
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
    $_ = $line;
    for $cpu (@stat_cpus)
    {   print STDOUT "cpu=\"$cpu\"\n";
	foreach $ss (min,max,ave,cnt)
	{   $out_line = "";
	    for $idx (0..$#col_cntl)
	    {
		if (!($line=~/$col_cntl[$idx]{re}/))
		{   $out_line = "problem with stats";
		    last;
		}
		else
		{   $data = $1;
		    if (!$opt_b)
		    {
			if ($col_cntl[$idx]{delta})
			{   $ssdata =sprintf( "%*s", length($data), $ss );
			}
			else { $ssdata =~ s/./ /g; }
			$out_line .= $ssdata; # delta will come after
		    }
		    if ($col_cntl[$idx]{delta})
		    {   if ($data =~ /^\s*\d+/o)
			{   if ($col_cntl[$idx]{"cnt$cpu"})
			    {   if ($ss eq ave) { $col_cntl[$idx]{"${ss}$cpu"} /= $col_cntl[$idx]{"cnt$cpu"}; }
				$delta = sprintf( "%*d", $delta_width, $col_cntl[$idx]{"${ss}$cpu"} );
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
		    if ($opt_b)
		    {
			if ($col_cntl[$idx]{delta})
			{   $data =sprintf( "%-*s", length($data), $ss );
			}
			else { $data =~ s/./ /g; }
			$out_line .= $data; # delta was before
		    }
		}
	    }
	    print STDOUT "$out_line\n";
	}
    }
}
