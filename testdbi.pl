#!/usr/bin/perl

use strict;
use warnings;

use DBI;
use DBD::mysql;

my $db=DBI->connect(
	"DBI:mysql:database=mysql;host=localhost",
	"username",
	"password"
) || die "Error connection to database: $!";

my $statement = $db->prepare("SELECT * from user");

$statement->execute();

while (my $ref = $statement->fetchrow_arrayref) {
	print $ref->[1] . "newln";
}

exit;
