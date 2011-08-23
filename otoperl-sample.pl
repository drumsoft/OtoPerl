# otoperlc Otoperl/Basic.pm first

my $trigger = 0;
my $triggerl = 0;
my $nn = 0;
my $nnl = 0;
my $f5 = 500;
my $freqr = 1000;
my $f6 = 1000;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v, @v, $k, $f, $amp);
	my $m = 4;
	my $f1 = OtoPerl::Basic::freq_bynote(70);
	my $f2 = OtoPerl::Basic::freq_bynote(63);
	my $f3 = OtoPerl::Basic::freq_bynote(34);

	for ($i = $size-1; $i >= 0; $i--) {

		# sine > (sine LFO > AM)
		my $v1 = 0.8 * OtoPerl::Basic::osc_sin( $frame, 1.3 ) * OtoPerl::Basic::osc_sin( $frame, $f2 );

		# square > (sine LFO > AM)
		my $v2 = 0.5 * OtoPerl::Basic::osc_sin( $frame, 0.3 ) * OtoPerl::Basic::osc_square( $frame, $f3 );

		# random note > sine > (saw envelope > AM)
		if ($trigger < $frame) {
			$freqr = OtoPerl::Basic::freq_bynote(OtoPerl::Basic::scalednote([0,4,5,7,11], int(rand(20)) + 10 ) );
			$triggerl = 6000 * ( int(rand(2)) + 1 );
			$trigger = $frame + $triggerl;
		}
		$amp  = ($trigger - $frame) / $triggerl;
		$amp  = $amp > 0 ? ($amp > 1 ? 1 : $amp) : 0;
		my $v4 = OtoPerl::Basic::osc_sin( $frame, $freqr ) > 0 ? $amp : -$amp;

		# random note > sine > (saw envelope > AM)
		if ($nn < $frame) {
			$nnl = 6000 * ( int(rand(2)) + 1 );
			$nn = $frame + $nnl;
		}
		$amp  = ($nn - $frame) / $nnl;
		$amp  = $amp > 0 ? ($amp > 1 ? 1 : $amp) : 0;
		my $v3 = $amp * (rand(2) - 1);

		# downstair note > sine
		if (0 == $frame % 12000) {
			$f5 = OtoPerl::Basic::freq_bynote(OtoPerl::Basic::scalednote([0,4,5,11,7], 48 - (($frame / 12000) % 24) ) );
		}
		my $v5 = 0.8 * OtoPerl::Basic::osc_sin( $frame, $f5 );

		# upstair note > square > (sine LFO > AM)
		if (0 == $frame % 24000) {
			$f6 = OtoPerl::Basic::freq_bynote(OtoPerl::Basic::scalednote([0,5,4,7,11], ($frame / 24000) % 16 + 10));
		}
		my $v6 = 0.4 * OtoPerl::Basic::osc_square( $frame, $f6 );
		my $pan = OtoPerl::Basic::osc_sin( $frame, 0.3 );

		# mix
		$w[$i + $size*0] = (    $v1+ (1-$pan)*$v2+ $v3+ $v4+0.2* $v5+ $v6*$pan)*0.3;
		$w[$i + $size*1] = (0.2*$v1+     $pan*$v2+ $v3+ $v4+    $v5+ $v6*(1-$pan))*0.3;
		$frame++;
	}
	return @w;
}
1;