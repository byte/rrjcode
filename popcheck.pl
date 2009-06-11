#!/usr/bin/perl
# check to see if your POP3 account is working or if your password is wonky

use Net::POP3;
usage(0) if "-h" eq $ARGV[0] || "-help" eq $ARGV[0];
usage(1) if 2 != $#ARGV;
($hostname, $account, $password) = @ARGV;

$handle = Net::POP3->new($hostname) or kill "Connection to POP3 host $hostname unable to be established. \n";

defined($handle->login($account, $password)) or kill "$hostname cannot authenticate ($account, $password) currently. \n";

$message_list = $handle->list or kill "Message list cannot be retrieved. \n";

foreach $item (keys %$message_list) {
$header = $handle->top($item);
print @$header;
}

sub usage {
print "Usage: 'popcheck HOSTNAME ACCOUNT PASSWORD',\n";
print "or 'popcheck -help', to see this message. \n";
exit ($_[0]);
}

