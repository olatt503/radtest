#!/usr/bin/perl -w

use strict;
use warnings FATAL => 'all';

use BerkeleyDB ;

my $db_file = $ENV{'SMSOTP_P_USRDBHOME'};

my $filename = $db_file . ".$$";
unlink $filename;

my $db = new BerkeleyDB::Hash
        -Filename => $filename,
        -Flags    => DB_CREATE
        or die "Cannot open file $filename: $! $BerkeleyDB::Error\n" ;

$db->db_put('administrator@directory.gmvl.de', 'thomas+test@glanzmann.de');
$db->db_put('@myradtest.com', '@gmail.com');

rename($filename, $db_file) || die;
