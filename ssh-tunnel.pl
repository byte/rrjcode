#!/usr/bin/perl
#
# Usage: ssh-tunnel.pl [daemon port] ssl-proxy port destination_host port
#
# This script can be used by ssh to traverse a www-proxy/firewall that
# supports the http CONNECT command. (Note: Properly setup proxies that allow
# CONNECT will only allow a connection through to SSL ports (443 and 563
# according to squid.))
#
# Example-- connect to host named "remote" outside of your firewall:
#
# $ ssh remote -o'ProxyCommand ssh-tunnel.pl www-proxy 8080 remote 443'
#
# Better yet, insert the ProxyCommand definition for host "remote" in
# your $HOME/.ssh/config file:
#
#      .
#      .
#    Host remote
#      ProxyCommand /usr/local/bin/ssh-tunnel.pl www-proxy 8080 %h %p
#      .
#      .
#
# Written by Urban Kaveus <urban@statt.ericsson.se>
#
# Modified by Colin Charles <byte@aeon.com.my> / http://www.bytebot.net/
# * Works behind MS Proxies now
# * For username/password, exporting the http_proxy details in a shell help
# * Nicer version:
# 	http://www.cpan.org/authors/id/A/AH/AHORNBY/ssltunnel-1.0.pl

use Socket;
use IO::Select;

$dport = shift if ( $#ARGV > 3 );
($sslproxy,$proxyport,$destination,$destport) = @ARGV;

die "Usage: $0 [daemon port] ssl-proxy port destination port\n"
	unless ( $sslproxy && $proxyport && $destination && $destport );

# set up log file
#open(LOG, ">>/tmp/ssh-tunnel.log") ||
#	die "unable to open logfile: $!";
#print LOG "====== ssh-tunnel started.\n";

# Set up network communication
$protocol = getprotobyname("tcp");

if ( $dport ) { # do "daemon" thing.
	socket(INC, PF_INET, SOCK_STREAM, $protocol) || die "socket:$!";
	setsockopt (INC, SOL_SOCKET, SO_REUSEADDR, pack ("l", 1)) || die "setsockopt:$!";
	bind(INC,sockaddr_in($dport,INADDR_ANY)) || die "bind: $!";
	listen(INC,1) || die "listen:$!";
	accept(OUT,INC) || die "accept:$!";
	$fhin = $fhout = \*OUT;
}
else { # STDIN/STDOUT used ...
	$fhin = \*STDIN;
	$fhout = \*STDOUT;
}

# connect to proxy server ...
socket (PROXY, PF_INET, SOCK_STREAM, $protocol) or
    die("Failed to create socket:$!");
connect (PROXY, sockaddr_in($proxyport,inet_aton($sslproxy))) or
    die("Failed to connect to $sslproxy port $proxyport:$!");

# Force flushing of socket buffers
foreach ( \*PROXY, $fhin, $fhout ) {
	select($_); $|=1;
}

# Send a "CONNECT" command to proxy:
print PROXY "CONNECT $destination:$destport HTTP/1.0\r\n\r\n";

# Wait for HTTP status code, bail out if you don't get back a 2xx code.
($status) = (split(/\s+/,<PROXY>))[1];

die "Received a bad status code \"$status\" from proxy server."
    if ( int($status/100) != 2 );

# Skip through remaining part of HTTP header (until blank line)
1 until ( <PROXY> =~ /^[\r\n]+$/ );

# Start copying packets in both directions.
$s = IO::Select->new($fhin,\*PROXY);

while ( 1 ) {
	foreach $fh ( $s->can_read(10) ) {
		exit unless ( defined($num = sysread($fh,$_,4096)) );
		exit if $num == 0;
		#$tstamp = localtime;
		#print LOG $tstamp, ": read ", $num, " bytes.\n";
		exit unless ( defined(syswrite( ((fileno($fh)==fileno($fhin))?
			PROXY:$fhout),$_,$num)) );
		#print LOG $tstamp, ": wrote ", $num, " bytes.\n";
	}
}
