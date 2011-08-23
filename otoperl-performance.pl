use strict;
use warnings;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v, @v, $k, $f, $amp, $freq);
	my $m = 4;
	my $l = 10000;
	for ($i = $size-1; $i >= 0; $i--) {
		for($k = 0; $k < $m; $k++) {
			$f = $frame + 25000 * $k;

			$freq = (($f / $l) % 24 + 1) * 55; #stair
			#$freq = ($k+1) * 55;              #no stair
			$v[$k] = sin( 3.1415 * 2 * $f * $freq / 48000 ); #sine
			#$v[$k] = (($f * $freq / 24000) & 1) - 0.5; #no sine

			$v[$k] = $v[$k] > 0 ? 0.5 : -0.5; #square

			$amp  = ($l - ($f % $l)) / $l; #amp
			$v[$k] *= $amp; #amp
		}
		$v = 0;
		$v += $_ foreach(@v);
		$v /= $m;

		$w[$i + $size*0] = $v;
		$w[$i + $size*1] = $v;
		$frame++;
	}
	return @w;
}
1;