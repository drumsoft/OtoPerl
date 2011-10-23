use strict;
use warnings;

our $frame;

our (@sndL, @sndR, $sndSize);

# read AIFF file and store in ARRAYs
sub readfile {
	my ($file, $L, $R) = @_;
	@sndL = ();
	@sndR = ();
	my $in;
	open $in, $file or return "cannot open $file";
	my ($ckid, $size, $i, $raw_sample);
	while(! eof $in) {
		read($in, $ckid, 4) or return 'cannot read ckID';
		if ($ckid eq 'FORM') {
			seek $in, 8, 1 or return 'cannot seek at FORM AIFF Chunk';
		}elsif ($ckid eq 'COMM') {
			read($in, $size, 4) or return 'cannot read size of COMM Chunk';
			$size = unpack 'N', $size;
			seek $in, $size, 1 or return 'cannot seek at COMM Chunk';
		}elsif ($ckid eq 'SSND') {
			read($in, $size, 4) or return 'cannot read size of SSND Chunk';
			$size = unpack 'N', $size;
			seek $in, 8, 1 or return 6;
			my $size = $size-8;
			for ($i = $size; $i > 0; $i -= 4) {
				read($in, $raw_sample, 2) or return 'cannot read a sample';
				push @$L, (unpack 's', reverse $raw_sample) / 32768;
				read($in, $raw_sample, 2) or return 'cannot read a sample';
				push @$R, (unpack 's', reverse $raw_sample) / 32768;
			}
			last;
		}else {
			read($in, $size, 4) or return 'cannot read size of other Chunk';
			$size = unpack 'N', $size;
			seek $in, $size, 1 or return 'cannot seek at other Chunk';
		}
	}
	close($in);
	return undef;
}

# read aiff file
# sample.aiff is singned 16 bit, 48kHz. placed in otoperld directory.
my $r = readfile('sample.aiff', \@sndL, \@sndR);
die($r) if $r;
$sndSize = @sndL;

$frame = 0;

# return sound data of sampled sound. will be overwritten.
sub tapeplay {
	# th means "Tape Header"
	my $th = shift;

	# looping on sample length
	$th %= $sndSize;

	($sndL[$th], $sndR[$th]);
}

# main callback
sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i);
	my $m = 4;
	for ($i = $size-1; $i >= 0; $i--) {
		# return sound data.
		my @v = tapeplay($frame);

		$w[$i + $size*0] = $v[0];
		$w[$i + $size*1] = $v[1];
		$frame++;
	}
	return @w;
}

1;