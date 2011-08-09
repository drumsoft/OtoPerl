package OtoPerl::Client;

use warnings;
use strict;

use LWP::UserAgent;

sub send {
	my $class = shift;
	my $options = shift;
	my $url = sprintf 'http://%s:%s/'
		, $options->{-host}, $options->{-port};
	
	foreach(@_) {
		_send($url, $_);
	}
}

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
		_say("[$_]: " . $response->content);
	}else{
		_say("[$_] ERROR (" . $response->status_line . "): " . $response->content);
	}
}

sub _say {
	print shift;
	print "\n";
}

1;
