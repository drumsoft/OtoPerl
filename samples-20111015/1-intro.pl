use strict;
use warnings;

our $frame;

my $freq = 0;
my %note2number = qw(c 0 c+ 1 d- 1 d 2 d+ 3 e- 3 e 4 f 5 f+ 6 g- 6 g 7 g+ 8 a- 8 a 9 a+ 10 b- 10 b 11 r -1);
my $base_width = 48000 * 60 * 4 / 130;
sub score_read {
	if ( $_ =~ /([cdefgabr][+-]?)(\d*(\.?))/i ) {
		my $length = int($base_width / ($2 ? $2 : 16));
		$length = int($length * 1.5) if $3;
		{
			number => $note2number{$1},
			length => $length
		}
	}
}

# MML like score for bass sound
my @score1 = map score_read, 
	qw(g8 r d8 r f8 r4. d8 f8 r f8 r8. g g f8 g r f8);

my $filter = OtoPerl::Basic::Filter->new();
$filter->set( 500 , 4 );

$frame = 0;
my $width = 48000;
my $width8 = $base_width / 8;
my $number = 0;
my $nstart = $frame;
my $nend = $frame;

my $env = 0;
my $amp = 0;

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i);
	my $m = 4;
	for ($i = $size-1; $i >= 0; $i--) {
		# bass sound: process notes
		my $ratio;
		if ($frame >= $nend) {
			$number++;
			$nstart = $frame;

			my $note = $score1[ $number % @score1 ];
			if ($note->{number} eq -1) {
				$freq = 0;
			}else{
				$freq = OtoPerl::Basic::freq_bynote(
					$note->{number} + 24 );
			}
			$width = $note->{length};
			$nend = $frame + $width;
		}
		$ratio = ($frame - $nstart) / $width;

		# bass sound: generate and filter
		my $v6;
		if ($freq == 0) {
			$v6 = 0;
		}else{
			$v6 = $frame * $freq / 48000;
			$v6 = (1 - $ratio) * (
				($v6 - int($v6)) > 0.5 ? 1 : -1
			);
			$filter->set( (1 - $ratio) * 300 + OtoPerl::Basic::osc_sin( $frame, 0.11 ) * 400 + 500 , 5 );
		}
		$v6 = 0.5 * $filter->val($v6);

		# bass sound: clipping
		$v6 = $v6 > 0.9 ? 0.9 : $v6 < -0.9 ? -0.9 : $v6;

		# rhythm sound: timing and length
		if ( $frame % $width8 == 0 ) {
			$amp = rand(1) * 0.3 + 0.1;
		}
		$env = $frame / $width8;

		# rhythm sound: generate 
		if (int($env) % 2) {
			$env = 1 - 8 * ($env - int($env));
			$env = 0 if $env < 0;
		}else{
			$env = 1 - ($env - int($env));
		}
		my $v3 = $env * $amp * (rand(2) - 1);

		# mix
		$w[$i + $size*0] = $v6 + $v3;
		$w[$i + $size*1] = $v6 + $v3;
		$frame++;
	}
	return @w;
}
1;