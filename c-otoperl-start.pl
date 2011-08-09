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
	for ($i = $size-1; $i >= 0; $i--) {
		$v = 0.8 * sin( 3.1415 * 2 * $frame * 440 / 48000 );
		$w[$i + $size*0] = $v;
		$w[$i + $size*1] = $v;
		$frame++;
	}
	return @w;
#	print $count++ . "\n";
}
