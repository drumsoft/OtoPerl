use strict;
use warnings;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v, @v, $k, $f, $amp);
	my $m = 4;
	for ($i = $size-1; $i >= 0; $i--) {
		# sine > square > (sine LFO > AM)
		$v = 0.8 * sin( 3.1415 * 2 * $frame * 440 / 48000 );
		$v = $v > 0 ? 0.5 : -0.5;
		$v *= sin( 3.1415 * 2 * $frame * 1 / 48000 );

		# sine > (sine LFO > AM)
		$v2 = 0.8 * sin( 3.1415 * 2 * $frame * 330 / 48000 );
		$v2 *= sin( 3.1415 * 2 * $frame * 1.3 / 48000 );

		# sine > square > (sine LFO > AM)
		$v3 = 0.8 * sin( 3.1415 * 2 * ($frame) * 60 / 48000 );
		$v3 = $v3 > 0 ? 0.5 : -0.5;
		$v3 *= sin( 3.1415 * 2 * $frame * 0.3 / 48000 );

		# random note > sine > (saw envelope > AM)
		if (0 == $frame % 12000) {
			$freqr = int(rand(16)) * 110;
		}
		$amp  = (12000 - ($frame % 12000)) / 12000;
		$amp  = $amp > 0 ? $amp : 0;
		$v4 = sin( 3.1415 * 2 * $frame * $freqr / 48000 );
		$v4 = $v4 > 0 ? $amp : -$amp;

		# downstair note > sine
		$freq = (24 - ($frame / 12000) % 24) * 55;
		$v5 = 0.8 * sin( 3.1415 * 2 * $frame * $freq / 48000 );

		# upstair note > sine > square > (sine LFO > AM)
		$freq = (($frame / 24000) % 16) * 55+110;
		$v6 = 0.8 * sin( 3.1415 * 2 * ($frame) * $freq / 48000 );
		$v6 = $v6 > 0 ? 0.4 : -0.4;
		$pan = sin( 3.1415 * 2 * $frame * 0.3 / 48000 );

		# mix
		$w[$i + $size*0] = $pan     * ($v+$v2+$v3+$v4+$v6*$pan)*0.3;
		$w[$i + $size*1] = (1-$pan) * ($v+$v3+$v5+$v6*(1-$pan))*0.3;
		$frame++;
	}
	return @w;
}
1;