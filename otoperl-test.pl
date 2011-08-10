use strict;
use warnings;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);
	my $pan = pan();
	for ($i = $size-1; $i >= 0; $i--) {
		$v = 0.8 * sin( 3.1415 * 2 * $frame * 440 / 48000 );
		$v = $v > 0 ? 0.5 : -0.5;
		$v *= sin( 3.1415 * 2 * $frame * 1 / 48000 );

		$v2 = 0.8 * sin( 3.1415 * 2 * $frame * 330 / 48000 );
		$v2 *= sin( 3.1415 * 2 * $frame * 1.3 / 48000 );

		$v3 = 0.8 * sin( 3.1415 * 2 * ($frame+50*$v2) * 60 / 48000 );
		$v3 = $v3 > 0 ? 0.5 : -0.5;
		$v3 *= sin( 3.1415 * 2 * $frame * 0.3 / 48000 );

		$freq = (($frame / 12000) % 10) * 110;
		$amp  = (12000 - ($frame % 12000)) / 12000;
		$v4 = sin( 3.1415 * 2 * $frame * $freq / 48000 );
		$v4 = $v4 > 0 ? $amp : -$amp;

		$freq = (($frame / 12000) % 24) * 55;
		$v5 = 0.8 * sin( 3.1415 * 2 * $frame * $freq / 48000 );

		#$v = ($frame / (48000/35)) % 2 ? $v : 0;
		$w[$i + $size*0] = $pan     * ($v+$v2+$v4)*0.4;
		$w[$i + $size*1] = (1-$pan) * ($v+$v3+$v5)*0.4;
		$frame++;
	}
	return @w;
}
1;