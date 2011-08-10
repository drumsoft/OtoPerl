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
		#$v = ($frame / (48000/35)) % 2 ? $v : 0;
		$w[$i + $size*0] = $pan     * $v;
		$w[$i + $size*1] = (1-$pan) * $v;
		$frame++;
	}
	(undef)->method();
	return @w;
}
1;