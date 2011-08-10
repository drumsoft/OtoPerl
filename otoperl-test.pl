use strict;
use warnings;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);
	for ($i = $size-1; $i >= 0; $i--) {
		$v = 0.5 * sin( 3.1415 * 2 * $frame * 440 / 48000 );

		$v *= sin( 3.1415 * 2 * $frame * 2 / 48000 );

		$w[$i + $size*0] = $v;
		$w[$i + $size*1] = $v;
		$frame++;
	}
	return @w;
}
1;