#!/usr/bin/perl
#   This file (trace_delta.pl) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jan  6, 2000. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: trace_delta.pl,v $
#   $Revision: 1.2 $
#   $Date: 2000-01-10 15:59:24 $


$USAGE = "\
usage: $0 [options] [files]
options:
-field <heading field> Specify which field to calculate delta for.
-col_output  <cols>    Specify colums to include in output (i.e: '5,0..4,6')
-delta_col   <cols>    Use *after* -c to specify which cols get deltas
-notimeStamp           Disable delta on default \"timeStamp\" heading.
                       This option, if used, must be first.
-re <re>               i.e: '/(begin|end)/ && !/947373119916951/'
";


$delta_width = 10;

$col_out_default = 1;
$delta[0]{heading} = 'timeStamp';
$delta_idx = 1;
#
#  PARSE OPTIONS
# 
while ($ARGV[0] =~ /^-/)
{   $_ = shift;
    if (/^-f/)
    {   $delta[$delta_idx]{heading} = shift;
	$delta_idx++;
    }
    elsif (/^-n/)
    {   $delta_idx = 0;
    }
    elsif (/^-[h?]/)
    {   die "$USAGE\n";
    }
    elsif (/^-c/)
    {   $col_out_opt = shift;
	$col_out_default = 0;
    }
    elsif (/^-d/)
    {   $delta_col_opt = shift;
	$delta_col_flag = 1;
    }
    elsif (/^-r/)
    {   $opt_re = shift;
	$opt_re_flag = 1;
    }
    else
    {   die "Unrecognized option: $_\n$USAGE\n";
    }
}

#
#   PROCESSING BASED ON OPTIONS
#

if ($col_out_default)
{   $column_out_re[0] = '(.*)';
}
else
{   eval "\@col_out = ($col_out_opt)";
    $idx = 0;
    foreach $col (@col_out)
    {   $column_out_re[$idx] = '\s+\S+' x $col . '(\s+\S+)';
	$idx++;
    }
}

#
#   FIRST LINE PROCESSING  -- it may be a header???
#

$heading = <>;
foreach $idx (0 .. $#column_out_re)
{   $heading =~ /$column_out_re[$idx]/;
    $line_out = $line_out . $1;
}

if ($delta_col_flag)
{   eval "\@delta_col = ($delta_col_opt)";
    $idx = 0;
    foreach $col (@delta_col)
    {   $delta[$idx]{re} = '(' . '\s+\S+' x $col . ')(\s+\S+)(.*)';
	$idx++;
    }
    foreach $idx (0 .. $#delta)
    {   if ($line_out =~ /$delta[$idx]{re}/)
	{   $first = $1;
	    $data = $2;
	    $rest = $3;
	    if ($data =~ /\d+/)
	    {   if (!$delta[$idx]{prev}) { $delta[$idx]{prev} = $data; }
		$delta = sprintf( "%*d", $delta_width, $delta[$idx]{prev}-$data );
		$line_out = "$first$data $delta$rest";
		$delta[$idx]{prev} = $data;
	    }
	    else
	    {   $space = sprintf( "%*s", ($delta_width+1), 'delta' );
		$line_out = "$first$data$space$rest";
	    }
	}
    }
    print STDOUT "$line_out\n";
}
else
{   foreach $idx (0 .. $#delta)
    {
	if ($line_out =~ /^((\s+\S+)*)(\s+$delta[$idx]{heading})(.*)/)
	{   $leader = $1;
	    $head = $3;
	    $rest = $4;
	    $dots = "." x length($leader);
	    $delta[$idx]{re} = "($dots\\s*)(\\d+)(.*)";
	    
	    $space = sprintf( "%*s", ($delta_width+1), 'delta' );
	    $line_out = "$leader$head$space$rest";
	}
	else
	{   die "\
delta heading \"$delta[$idx]{heading}\" not found.\
Use -n OR -d option or check -f options\n";
	}
    }
    print STDOUT "$line_out\n";
}

#
#   NOW, PROCESS THE REST OF THE LINES
#


while (<>)
{   if ($opt_re_flag) { next unless eval ($opt_re); }
    chop; $line_in = $_;
    $line_out = '';
    foreach $idx (0 .. $#column_out_re)
    {   if ($line_in =~ /$column_out_re[$idx]/)
	{   $line_out = $line_out . $1;
	}
	elsif ($idx == 0)
	{   $line_out = $line_in; # just pass along the whole line
	    break;
	}
    }
    foreach $idx (0 .. $#delta)
    {   if ($line_out =~ /$delta[$idx]{re}/)
	{   $first = $1;
	    $data = $2;
	    $rest = $3;
	    if (!$delta[$idx]{prev}) { $delta[$idx]{prev} = $data; }
	    $delta = sprintf( "%*d", $delta_width, $delta[$idx]{prev}-$data );
	    $line_out = "$first$data $delta$rest";
	    $delta[$idx]{prev} = $data;
	}
    }
    print STDOUT "$line_out\n";
}
