#!/usr/bin/perl -w

use strict;
use warnings FATAL => 'all';

my ($domain, $username) = split(/\\/, $ARGV[0]);
my $password = $ARGV[1];

if ($domain eq "directory") {
        $domain = 'myradtest.com';
}

system("/usr/bin/ntlm_auth --request-nt-key --domain=${domain} --username=\Q${username}\E --password=\Q${password}\E");

exit($? >> 8);
