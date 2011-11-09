use strict;
use warnings;

our $frame;

my $osc1   = OtoPerl::Basic::OSC->new(
	            OtoPerl::Basic::freq_bynote(63)
	            , undef, $frame );

my $osc2   = OtoPerl::Basic::OSC->new(
	            OtoPerl::Basic::freq_bynote(63)
	            , undef, $frame );

my $delay   = OtoPerl::Basic::Delay->new();
$delay->set(0.5);

my $freq = 0;
my $scale = [qw(0 2 4 5 7 9 11)];
my $scale2 = [qw(0 2 4 7 9)];

my $width = $sample_rate * 6;
my $number = 0;
my $nstart = $frame;
my $nend = $frame;

my $change1 = 32;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i);
	my $m = 4;
	for ($i = 0; $i < $size; $i++) {
		my $ratio;
		if ($frame >= $nend) {
			$number++;
			$nstart = $frame;
			if ($number <= $change1) {
				$freq = OtoPerl::Basic::freq_bynote(OtoPerl::Basic::scalednote($scale, $number)+36);
				$width = int($width * 0.7);
			}else{
				$width = $sample_rate / 12;
				$freq = OtoPerl::Basic::freq_bynote(OtoPerl::Basic::scalednote($scale2, $number%32)+24);
			}
			$nend = $frame + $width;
		}
		$ratio = ($frame - $nstart) / $width;

		my ($v6, $amp);
		if ($number <= $change1) {
			$osc2->freq( 1600+1500*OtoPerl::Basic::osc_sin( $frame, 0.11 ) );
			$osc1->freq( 
				$freq + 
				200*OtoPerl::Basic::osc_sin( $frame, 0.07 )
				 * $osc2->next() 
			);
			$amp = 0.8 * $ratio + 0.2 ;
			$v6 = $amp * ( $osc1->next() > OtoPerl::Basic::osc_sin( $frame, 0.13 ) ?1:-1 );
		}else{
			$osc2->freq( 1600+1500*OtoPerl::Basic::osc_sin( $frame, 0.11 ) );
			$osc1->freq( 
				$freq + 
				200*OtoPerl::Basic::osc_sin( $frame, 100 )
				 * $osc2->next() 
			);
			$v6 = ( $osc1->next() > OtoPerl::Basic::osc_sin( $frame, 5 ) ?1:-1 );
		}

		$v6 += 0.6 * $delay->output();
		$delay->input($v6);
		$v6 *= 0.6;
		$w[$i + $size*0] = $v6;
		$w[$i + $size*1] = $v6;
		$frame++;
	}
	return @w;
}
1;