use strict;
use warnings;

my $frame;

sub perl_render_init {
	$frame = 0;
}

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);
	my $pan = pan();
	for ($i = $size-1; $i >= 0; $i--) {
		$v = 0.8 * sin( 3.1415 * 2 * $frame * 440 / 48000 );
		$w[$i + $size*0] = $pan     * $v;
		$w[$i + $size*1] = (1-$pan) * $v;
		$frame++;
	}
	return @w;
}

sub pan{0.5}

