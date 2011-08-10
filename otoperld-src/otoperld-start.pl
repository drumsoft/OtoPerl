# OtoPerl - live sound programming environment with Perl.
# otoperld-start.pl - startpoint for OtoPerl.

#   OtoPerl - live sound programming environment with Perl.
#   Copyright (C) 2011- Haruka Kataoka
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

use strict;
use warnings;

my $frame;

sub perl_render_init {
	$frame = 0;
}

sub perl_render {
	my $size = shift;
	my $channels = shift;
	my (@w, $i, $v);
	for ($i = $size-1; $i >= 0; $i--) {
		$v = 0.5 * sin( 3.1415 * 2 * $frame * 440 / 48000 );
		$w[$i + $size*0] = $v;
		$w[$i + $size*1] = $v;
		$frame++;
	}
	return @w;
}

