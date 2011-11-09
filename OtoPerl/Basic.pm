package OtoPerl::Basic;

my $PI2 = 3.14159265 * 2;
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
		y2 => 0,
		freq_unit => $PI2 / $sample_rate
	};
	return bless $self, $class;
}

sub set {
	my ($v, $freq, $q) = @_;

	my $w0 = $freq * $v->{freq_unit};
	my $alpha = sin($w0) / (2 * $q);
	my $cs = cos($w0);

	my $b1 =   1 - $cs;
	my $b0 = $b1 / 2;

	$v->{a0} = 1 + $alpha; #$a0;
	$v->{k1} = $b0; #$b0;
	$v->{k2} = $b1; #$b1;
	$v->{k3} = $b0; #$b2;
	$v->{k4} =-2 * $cs ; #$a1;
	$v->{k5} = 1 - $alpha ; #$a2;
}

sub val {
	my $v = shift;
	$v->{x2} = $v->{x1};	$v->{x1} = $v->{x};	$v->{x} = shift;
	$v->{y2} = $v->{y1};	$v->{y1} = $v->{y};

	$v->{y} = ($v->{k1}*$v->{x} + $v->{k2}*$v->{x1} + $v->{k3}*$v->{x2}
                               - $v->{k4}*$v->{y1} - $v->{k5}*$v->{y2}) / $v->{a0};
}

package OtoPerl::Basic::Delay;

sub new {
	my ($class) = @_;
	my $self = {
		size => 0,
		v => []
	};
	return bless $self, $class;
}

sub set {
	my ($self, $time) = @_;
	$self->{size} = int($sample_rate * $time);
}

sub output {
	my ($self) = @_;
	@{$self->{v}} >= $self->{size} ? shift @{$self->{v}} : 0;
}

sub input {
	my ($self, $v) = @_;
	push @{$self->{v}}, $v;
}

1;
