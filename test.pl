print "Let's go!\n";

package OtoPerl::Server;

$render = sub {
	my $self = shift;
	my $size = shift;
	my $channels = shift;
	my $frame = $self->{frame};
	my (@w, $i, $v);
	for ($i = $size-1; $i >= 0; $i--) {
		$v = 0.8 * sin( 3.1415 * 2 * $frame * 440 / 48000 );
		$w[$i + $size*0] = $v;
		$w[$i + $size*1] = $v;
		$frame++;
	}
	$self->{frame} = $frame;
	return \@w;
};

1;
