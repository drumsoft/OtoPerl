package OtoPerl::Basic;

my $PI2 = 3.14159265 * 2;
my $sample_rate = 48000;
my $freq_tuning = 440;

sub default_wave {
	sin(shift);
}

my $osc_sin_fact = $PI2 / $sample_rate;
sub osc_sin { # ($frame, $freq)
	sin( $osc_sin_fact * (shift) * (shift) );
}

my $osc_square_fact = 2 / $sample_rate;
sub osc_square { # ($frame, $freq)
	int((shift) * (shift) * $osc_square_fact) & 1 ? 1 : -1;
}

sub freq_bynote {
	int( $freq_tuning * (2**( ((shift)-69)/12 )) );
}

#scalednote([0,2,4,5,7,9,11], $note) for C
#scalednote([1,3,6,8,10], $note)
sub scalednote {
	my $s = shift;
	my $n = shift;
	$s->[$n % @$s] + 12 * int($n / @$s);
}


package OtoPerl::Basic::OSC;

sub new {
	my ($class, $freq, $function, $frame) = @_;
	my $self = bless {
		frame => $frame,
		phase => 0,
		function => $function || \&OtoPerl::Basic::default_wave
	}, $class;
	$self->freq($freq);
	return $self;
}

sub freq {
	my ($self, $freq) = @_;
	if (defined $freq) {
		$self->{freq} = $freq;
		$self->{dphase} = $freq * $PI2 / $sample_rate;
		$self;
	}else{
		$self->{freq};
	}
}

sub val {
	my ($self, $frame) = @_;
	$self->{phase} += $self->{dphase} * ($frame - $self->{frame});
	$self->{frame} = $frame;
	$self->{function}->( $self->{phase} );
}

sub next {
	my ($self) = @_;
	$self->{phase} += $self->{dphase};
	$self->{frame}++;
	$self->{function}->( $self->{phase} );
}


package OtoPerl::Basic::EG;

sub new {
	my ($class, $freq, $function) = @_;
	my $self = {
		freq => $freq,
		function => $function || \&OtoPerl::Basic::osc_sin
	};
	return bless $self, $class;
}

sub val {
	my ($self, $frame) = @_;
	$self->{function}->($frame, $self->{freq});
}


package OtoPerl::Basic::Filter;

sub new {
	my ($class) = @_;
	my $self = {
		x => 0,
		x1 => 0,
		x2 => 0,
		y => 0,
		y1 => 0,
		y2 => 0
	};
	return bless $self, $class;
}

sub set {
	my ($v, $freq, $q) = @_;

	my $w0 = $PI2 * $freq / $sample_rate;
	my $alpha = sin($w0) / (2 * $q);
	my $cs = cos($w0);

	my $b1 =   1 - $cs;
	my $b0 = $b1 / 2;
	my $b2 = $b0;
	my $a0 =   1 + $alpha;
	my $a1 =  -2 * $cs;
	my $a2 =   1 - $alpha;

	$v->{k1} = $b0/$a0;
	$v->{k2} = $b1/$a0;
	$v->{k3} = $b2/$a0;
	$v->{k4} = $a1/$a0;
	$v->{k5} = $a2/$a0;
}

sub val {
	my $v = shift;
	$v->{x2} = $v->{x1};	$v->{x1} = $v->{x};	$v->{x} = shift;
	$v->{y2} = $v->{y1};	$v->{y1} = $v->{y};

	$v->{y} = $v->{k1}*$v->{x} + $v->{k2}*$v->{x1} + $v->{k3}*$v->{x2}
                               - $v->{k4}*$v->{y1} - $v->{k5}*$v->{y2};
}

1;
