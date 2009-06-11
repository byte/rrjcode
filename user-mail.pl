#!/usr/bin/perl 
# turn on -w for warnings, if need be

# Written by Colin Charles
# byte@aeon.com.my
# Tue Sep 17 03:30:13 EST 2002
# Version 2 last updated Tue Sep 17 13:32:22 EST 2002

# check command line args if need be
if (@ARGV!=3) {
	die "Usage: $0 email \"name\" username\n";
}

$USER_EMAIL=$ARGV[0];
$USERS_REAL_NAME=$ARGV[1];
$USERNAME=$ARGV[2];

open WHOAMI, "whoami|" or die "can't do a who am i - check if software is installed\n";

#read filehandle, and store it in a variable
while(<WHOAMI>) {
	chomp($whoami=$_);
}

#concantetate to get a nice e-mail address
$EMAIL_CREATOR=$whoami . "\@yoyo.its.monash.edu.au";

# print $EMAIL_CREATOR; #debug

open PASSWD, "/etc/passwd" or die "can't open password file";
while(<PASSWD>) {
	($username,$crypted,$uid,$gid,$name) = split /:/; #format of passwd file
	if ($username eq $whoami) { #check for equality
# print $name; #debug
		$CREATOR_NAME=$name;
	}
}

$YOYO="accounts\@yoyo.its.monash.edu.au";

open MAIL, "|/usr/lib/sendmail -t -f $EMAIL_CREATOR" or die "sendmail couldn't be opened";

print MAIL <<EOF; # print till EOF reached
To: $USER_EMAIL
From: $EMAIL_CREATOR
Subject: Creation of new yoyo account
Cc: $YOYO

Hi there $USERS_REAL_NAME,

Welcome to yoyo. Your request to create an account has been successful.  Here are some details:

Login: yoyo.its.monash.edu.au
Username: $USERNAME
Password: as you provided on the form

Please remember to change your password (as you will be prompted to) upon the first login to yoyo.

It is recommended that you use an SSH client to login to yoyo, as telnet is not as secure - clients are available freely on the Internet, for all different operating systems.

We hope you have fun using yoyo. And if you have any problems, please do not hesitate to contact the Accounts group (accounts\@yoyo.its.monash.edu.au) or even me ($EMAIL_CREATOR), and we'll get back to you as soon as possible.

Have fun! 

Regards,
$CREATOR_NAME.

EOF
close(MAIL);
# clean up
close(WHOAMI);
close(PASSWD);

