#!/usr/bin/perl
#   This file (trace_delta.pl) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jan  6, 2000. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: trace_delta.pl,v $
$version = '$Revision: 1.3 $';
#   $Date: 2000/01/11 02:02:52 $


$USAGE = "\
$version
usage:
       $0 [options] [files]...

The default col_spc is 'R0'

general_options:
-cpu <col_spec>     column for cpu field.
-c   <cols_spec>    columns to include in output
-d   <cols_spec>    columns 
-dc  <cols_spec>
-re  <re>               i.e: '/(begin|end)/ && !/947373119916951/'

cols_spec examples:
   CPU,msg
   3..5,0,11,Rest

defaults:
-d timeStamp
";

$delta_width = 10;

#
#   RECORD OPTIONS
#
while ($ARGV[0] =~ /^-/)
{   $_ = shift;
    if    (/^-[h?]/)
    {   die "$USAGE\n";
    }
    elsif (/^-dc/)
    {   $opt_dc = shift;
    }
    elsif (/^-d/)
    {   $opt_d = shift;
    }
    elsif (/^-c/)
    {   $opt_c = shift;
    }
    elsif (/^-cpu/)
    {   $opt_cpu = shift;
    }
    elsif (/^-v/)
    {   $opt_v = 1;
    }
    else
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

sub col_spec_to_re
{   $spec = shift @_;
    eval "\$col_spec = \$${spec}_col_spec";

    if ($col_spec =~ /\d+/)
    {   if ($col_spec == 0)
	{   $re = '(\s+\S+)';
	    $line =~ /$re/; $data = $1;
	    $re = '(' . '.' x length($data) . ')'
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

if ("$opt_dc")
{   # we need a cpu spec
    if ("$opt_cpu") { $cpu_col_spec = $opt_cpu;}
    else            { $cpu_col_spec = CPU;}

    # refine cpu_re
    &col_spec_to_re( cpu );

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


sub process_line
{   $out_line = "";
    for $idx (0..$#col_cntl)
    {   if (!($line=~/$col_cntl[$idx]{re}/))
	{   $out_line = $line;
	    last;
	}
	$data = $1;
	$out_line = $out_line . $data;
	if ($col_cntl[$idx]{delta})
	{   if ($data =~ /\d+/)
	    {   if ($col_cntl[$idx]{delta_cpu})
		{   $line =~ /$cpu_re/o;
		    $cpu = $1;
		}
		else
		{   $cpu = 0;  # something consistant for non-cpu specific delta
		}
		if (!$col_cntl[$idx]{"prev$cpu"}) { $col_cntl[$idx]{"prev$cpu"} = $data; }
		$delta = sprintf( "%*d", $delta_width, $col_cntl[$idx]{"prev$cpu"}-$data );
	    }
	    elsif ($data =~ /^-+$/)
	    {   $delta = sprintf( "%s", '-' x $delta_width );
	    }
	    else
	    {   $delta = sprintf( "%*s", $delta_width, 'delta' );
	    }
	    $out_line = $out_line . $delta;
	}
    }
    print STDOUT "$out_line\n";
}

&process_line;

while (<>)
{   if ("$opt_re") { next unless eval ($opt_re); }
    $line = $_;
    &process_line;
}
