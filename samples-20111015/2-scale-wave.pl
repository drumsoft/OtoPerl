my $PI2 = 3.14159265 * 2;

sub wave {
	my $phase = shift;

#	sin($phase);

	# Square wave
	($phase - $PI2*int($phase / $PI2)) / $PI2 > 0.5 ? 1 : -1;

	# Sawtooth wave
#	2 * ($phase - $PI2*int($phase / $PI2)) / $PI2 - 1;

	# Mute
#	0;
}

1;
