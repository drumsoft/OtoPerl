use strict;
use warnings;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);
	for ($i = $size-1; $i >= 0; $i--) {
		$w[$i + $size*0] = 0;
		$w[$i + $size*1] = 0;
		$frame++;
	}
	return @w;
}
1;