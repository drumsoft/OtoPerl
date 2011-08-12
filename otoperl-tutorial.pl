use strict;
use warnings;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);

	for ($i = $size-1; $i >= 0; $i--) {
		# generate sine wave
		$v = 0.5 * sin( 3.1415 * 2 * $frame * 440 / 48000 );

		# transform it to square wave
		# $v = $v > 0 ? 0.5 : -0.5;

		# amp modulation by low frequency sine wave
		$v *= sin( 3.1415 * 2 * $frame * 2 / 48000 );

		$w[$i + $size*0] = $v;
		$w[$i + $size*1] = $v;
		$frame++;
	}

	# put frames by reversed order because returned list is treated as stack.
	return @w;
}
1;