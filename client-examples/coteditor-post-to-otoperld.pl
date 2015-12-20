#!/usr/bin/perl
# %%%{CotEditorXInput=Selection}%%%

use strict;
use warnings;
use lib '..';
use OtoPerl::Client;

my $host = '127.0.0.1';
my $port = 14610;
my $notifier_bin = '/usr/local/bin/terminal-notifier';
my $notifier_title = 'OtoPerl';

my @results = OtoPerl::Client->send({
	-host => $host,
	-port => $port,
}, ('-'));

my $r;
if (defined $results[0]) {
	$r = system qq{'$notifier_bin' -message '$results[0]' -title '$notifier_title'};
} else {
	$r = system qq{'$notifier_bin' -message 'ok' -title '$notifier_title'};
}
die "terminal-notifier error: $r." if $r;
