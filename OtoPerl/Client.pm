package OtoPerl::Client;

use warnings;
use strict;

use LWP::UserAgent;

# ---------------------------------------- public
sub send {
	my $class = shift;
	my $options = shift;
	my $url = sprintf 'http://%s:%s/'
		, $options->{-host}, $options->{-port};
	
	map { _send($url, $_) } @_;
}

# ---------------------------------------- private
sub _send {
	my $url = shift;
	my $file = shift;
	local $/ = undef;

	my $in;
	open $in, $file;
	my $code = <$in>;
	close $in;

	my $ua = LWP::UserAgent->new;
	$ua->timeout(10);
	my $response = $ua->post($url, Content => $code);

	if ($response->is_success) {
		return undef;
	}else{
		return $response->status_line . "\n" . $response->content;
	}
}

# ---------------------------------------- utilities
sub read_options {
	my $defaults = shift;
	my $arglist = shift;
	while ( $_ = shift @$arglist ) {
		if ($_ =~ /^\-\w+$/) {
			exists $defaults->{$_} or die "unknown option '$_'.";
			@$arglist or die "no value for option '$_'.";
			$defaults->{$_} = shift @$arglist;
		}else{
			unshift @$arglist, $_;
			return $defaults;
		}
	}
	@$arglist or die "no input files.";
}

sub test_input_files {
	my $arglist = shift;
	@$arglist or die "no input files.";
	foreach (@$arglist) {
		next if $_ eq '-';
		-r $_ or die "input file '$_' not exists or readable.";
	}
}

1;
