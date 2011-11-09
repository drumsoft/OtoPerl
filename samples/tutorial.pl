use strict;
use warnings;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);

	for ($i = 0; $i < $size; $i++) {
		# generate sine wave
		$v = 0.5 * sin( 3.1415 * 2 * $frame * 440 / $sample_rate );

		# transform it to square wave
		#$v = $v > 0 ? 0.5 : -0.5;

		# amp modulation by low frequency sine wave
		#$v *= sin( 3.1415 * 2 * $frame * 2 / $sample_rate );

		for (my $c = 0; $c < $channels; $c++) {
			$w[$i + $size * $c] = $v;
		}
		$frame++;
	}

	# put frames by reversed order because returned list is treated as stack.
	return @w;
}
1;