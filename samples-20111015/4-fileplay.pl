our $frame;
our (@sndL, @sndR, $sndSize);

my ($loop_th, $loop_start, $loop_length);
my $loop_base = int(48000 * 60 / 130 / 16);
my $loop_switch = $loop_base * 32;


sub tapeplay {
	# th means "Tape Header"
	my $th = shift;

	# 1. FM: Modulate position of the tape header.
	$th = int($th + OtoPerl::Basic::osc_sin( $frame, 0.5 ) * 4000);
	$th %= $sndSize;
	return ($sndL[$th], $sndR[$th]);

	# 2. Chorus: mix FM sound and original sound.
#	my $th2 = int($th + OtoPerl::Basic::osc_sin( $frame, 1 ) * 80);
#	$th  %= $sndSize;
#	$th2 %= $sndSize;
#	return ( ($sndL[$th2] + $sndL[$th]) * 0.6, 
#	         ($sndR[$th2] + $sndR[$th]) * 0.6 );

	# 3. random length loops.
#	if ($th % $loop_switch == 0 || ! defined $loop_start) {
#		$loop_start = $th;
#		$loop_length = $loop_base * (2 ** int(rand(6)));
#	}
#	$loop_th = ($th - $loop_start) % $loop_length + $loop_start;
#	$loop_th = $loop_th % $sndSize;
#	return ($sndL[$loop_th], $sndR[$loop_th]);

}

1;
