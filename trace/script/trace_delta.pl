#!/usr/bin/perl
#   This file (trace_delta.pl) was created by Ron Rechenmacher <ron@fnal.gov> on
#   Jan  6, 2000. "TERMS AND CONDITIONS" governing this file are in the README
#   or COPYING file. If you do not have such a file, one can be obtained by
#   contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
#   $RCSfile: trace_delta.pl,v $
#   $Revision: 1.1 $
#   $Date: 2000/01/06 23:48:52 $


$USAGE = "\
usage: $0 [options] [files]
options:
-field <heading field> Specify which field to calculate delta for.
-notimeStamp           Disable delta on default \"timeStamp\" heading.
";


$delta_width = 10;

$delta[0]{heading} = 'timeStamp';
$idx = 1;
while ($ARGV[0] =~ /^-/)
{   $_ = shift;
    if (/^-f/)
    {   $delta[$idx]{heading} = shift;
	$idx++;
    }
    elsif (/^-[h?]/)
    {   die "$USAGE\n";
    }
    else
    {   die "Unrecognized option: $_\n$USAGE\n";
    }
}

$heading = <>;
foreach $idx (0 .. $#delta)
{
    if ($heading =~ /^((\s+\S+)*)(\s+$delta[$idx]{heading})(.*)/)
    {   $leader = $1;
	$head = $3;
	$rest = $4;
	$dots = "." x length($leader);
	$delta[$idx]{re} = "($dots\\s*)(\\d+)(.*)";

	$space = sprintf( "%*s", ($delta_width+1), 'delta' );
	$heading = "$leader$head$space$rest";
    }
    else
    {   die "delta heading \"$delta[$idx]{heading}\" not found";
    }
}
print STDOUT "$heading\n";

$line = <>; print STDOUT "$line";
$line = <>; print STDOUT "$line";


# initialize "prev"
$line = <>;
foreach $idx (0 .. $#delta)
{   
    if ($line =~ /$delta[$idx]{re}/)
    {   $first = $1;
	$data = $2;
	$rest = $3;
	$delta = sprintf( "%*d", $delta_width, 0 );
	$line = "$first$data $delta$rest";
	$delta[$idx]{prev} = $data;
    }
    else
    {   print STDOUT "re no match\n";
    }
}
print STDOUT "$line\n";


while (<>)
{
    $line = $_;
    foreach $idx (0 .. $#delta)
    {   
	if ($line =~ /$delta[$idx]{re}/)
	{   $first = $1;
	    $data = $2;
	    $rest = $3;
	    $delta = sprintf( "%*d", $delta_width, $delta[$idx]{prev}-$data );
	    $line = "$first$data $delta$rest";
	    $delta[$idx]{prev} = $data;
	}
	else
	{   print STDOUT "re no match\n";
	}
    }
    print STDOUT "$line\n";
}
