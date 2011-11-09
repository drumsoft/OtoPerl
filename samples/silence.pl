use strict;
use warnings;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);
	for ($i = 0; $i < $size; $i++) {
		for (my $c = 0; $c < $channels; $c++) {
			$w[$i + $size * $c] = 0;
		}
		$frame++;
	}
	return @w;
}
1;