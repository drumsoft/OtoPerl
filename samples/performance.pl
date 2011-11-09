use strict;
use warnings;

my $length = 10000 * $sample_rate / 48000;
my $gap = 25000 * $sample_rate / 48000;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v, @v, $k, $f, $amp, $freq);
	my $m = 6;
	for ($i = 0; $i < $size; $i++) {
		for($k = 0; $k < $m; $k++) {
			$f = $frame + $gap * $k;

			$freq = (($f / $length) % 24 + 1) * 55; #stair
			#$freq = ($k+1) * 55;              #no stair
			$v[$k] = sin( 3.1415 * 2 * $f * $freq / $sample_rate ); #sine
			#$v[$k] = (($f * $freq / 24000) & 1) - 0.5; #no sine

			$v[$k] = $v[$k] > 0 ? 0.5 : -0.5; #square

			$amp  = ($length - ($f % $length)) / $length; #amp
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