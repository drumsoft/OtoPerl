my $osc1   = OtoPerl::Basic::OSC->new(
	            OtoPerl::Basic::freq_bynote(63)
	            , sub{wave(shift)}, $frame );

my $scale;
$scale = [0,4,7,9];

my @b;
my $delay = 18000;
my $delay_fb = 0.6;

my $PI2 = 3.14159265 * 2;

#generater subroutine: called from $osc1 and will be overwritten.
sub wave {
	sin(shift);
}

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v, $f);

	for ($i = $size-1; $i >= 0; $i--) {
		# process note
		if (0 == $frame % 24000) {
			$f = OtoPerl::Basic::freq_bynote(OtoPerl::Basic::scalednote($scale, ($frame / 24000) % 16 + 16));
		}

		# generate wave
		$osc1->freq( $f );
		my $v = $osc1->next();

		# process delay
		my $d = @b > $delay ? shift @b : 0;
		$v += $delay_fb * $d;
		push @b, $v;

		# mix
		$w[$i + $size*0] = 0.35 * $v;
		$w[$i + $size*1] = 0.35 * $v;
		$frame++;
	}
	return @w;
}
1;