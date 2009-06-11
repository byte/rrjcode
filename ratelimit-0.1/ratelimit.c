/*
 * Purpose: drops packets when the ppmin rate is too high on a per
 *          client IP basis.
 *          Client IP ppmin code originally taken from FreeBSD's inetd.c
 * Example usage: ipfw add 1 divert 1 tcp from any to any in 80 setup
 */

/* PORTIONS ARE Copyright (c) 1983, 1991, 1993, 1994
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */




#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <syslog.h>
#include <time.h>


#define MAX_PACKET 16384   /* Largest packet size expected, after reassembly */
#define DIVERTPORT 1	   /* The divert port specified in ipfw command */
#define MAXCPM 50	   /* Rate at which 50% of packets are dropped  */
#define MAXCPM2 50	   /* Rate at which 100% of packets are dropped  */
#define LOG_PAUSE  20      /* seconds to wait between syslog() calls */

void IpChecksum(struct ip *);
void TcpChecksum(struct ip *);
int cpmip(struct ip *);

int main( int argc, char **argv )
{
	struct sockaddr_in sin;
	struct ip *ip;
        struct tcphdr*  tcphdr;
	int sockfd, sinlen, flags, bytes, divertport;
	size_t len;
	char buf[MAX_PACKET];

	openlog("ratelimit", LOG_CONS, LOG_DAEMON);

	sinlen=sizeof(sin);
	flags=0;
	len=MAX_PACKET - 1 ;
	ip=(struct ip *)buf;
	divertport=DIVERTPORT;

	/* Firstly, obtain a socket */
	if( (sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_DIVERT)) < 0 ) {
		perror( "divertd: can't open divert socket");
		exit(-1);
	}
	/* Now bind to the socket, setting appropriate parameters */
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = PF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(divertport);

	if( bind(sockfd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
		perror( "divertd: can't bind divert socket");
		exit(-1);
	}

	/* and loop read/print/write */
	while( 1 ) {
		bytes=recvfrom( sockfd, buf, len, flags, 
				(struct sockaddr *) &sin, &sinlen);

		/* drop packet if rate exceeded */
		if (cpmip(ip) < 0) 
			continue;

		/* The kernel zeroes the checksum before the packet
		 * is diverted, so we have to recalculate.
		 */
        	if (ip->ip_p == IPPROTO_TCP) {
               	  tcphdr = (struct tcphdr*) ((char*) ip + (ip->ip_hl << 2));
		  tcphdr->th_sum = 0;
		  (void)TcpChecksum(ip);
		}
		ip->ip_sum = 0;
  		IpChecksum(ip);

		/* Send the packet on its way. */
		bytes=sendto( sockfd, buf, bytes, flags, 
				(struct sockaddr *) &sin, sinlen);
	}

}



#define CPMHSIZE        256
#define CPMHMASK        (CPMHSIZE-1)
#define CHTGRAN         10  /* seconds per timeslice */
#define CHTSIZE         24   /* number of timeslices  */

typedef struct CTime {
        unsigned long   ct_Ticks;
        int             ct_Count;
} CTime;

typedef struct CHash {
        struct in_addr  ch_Addr;           /* Client IP                */
        time_t          ch_LTime;          /* Last time for this event */ 
        time_t          ch_LogTime;        /* Last time event logged   */ 
        CTime           ch_Times[CHTSIZE]; /* count events in each     */
                                           /* timeslice                */
} CHash;

CHash   CHashAry[CPMHSIZE];

int
cpmip(ip)
        struct ip *ip;
{
        int r = 0;


        time_t t = time(NULL);
        int hv = 0xABC3D20F;
        int i;
        int cnt = 0;
        int rate;
        CHash *chBest = NULL;
        unsigned int ticks = t / CHTGRAN;   /* current time in ticks */

	/* Work out hash of src IP */
        {
                char *p;
                int i;

                for (i = 0, p = (char *)&ip->ip_src;
                    i < sizeof(ip->ip_src);
                    ++i, ++p) {
                        hv = (hv << 5) ^ (hv >> 23) ^ *p;
                }
                hv = (hv ^ (hv >> 16));
        }

	/* Find IP in array, or allocate new bucket */
        for (i = 0; i < 5; ++i) {
                CHash *ch = &CHashAry[(hv + i) & CPMHMASK];

                if (ip->ip_src.s_addr == ch->ch_Addr.s_addr) {
                        chBest = ch;
                        break;
                }
                if (chBest == NULL || ch->ch_LTime == 0 ||
                    ch->ch_LTime < chBest->ch_LTime) {
                        chBest = ch;
                }
        }

	/* if starting new bucket, free() service identifier
         *  and put in the new service identifier, and zero
         *  the counter array 
	 */
        if (ip->ip_src.s_addr != chBest->ch_Addr.s_addr) {
                chBest->ch_Addr = ip->ip_src;
                bzero(chBest->ch_Times, sizeof(chBest->ch_Times));
			chBest->ch_LogTime = 0;
        }


	/* Set last time used to now */
        chBest->ch_LTime = t;
        {
		/* reset count in this time chunk if existing */
		/* data older than appropriate                */
                CTime *ct = &chBest->ch_Times[ticks % CHTSIZE];
                if (ct->ct_Ticks != ticks) {
                        ct->ct_Ticks = ticks;
                        ct->ct_Count = 0;
                }
		/* increment count in this time chunk */
                ++ct->ct_Count;
        }


	/* Count up all connections in last period */
        for (i = 0; i < CHTSIZE; ++i) {
                CTime *ct = &chBest->ch_Times[i];
                if (ct->ct_Ticks <= ticks &&
                    ct->ct_Ticks >= ticks - CHTSIZE) {
                        cnt += ct->ct_Count;
                }
        }


	/* return -1 if rate exceeded */
	/* rate = cnt * (CHTSIZE * CHTGRAN) / 60; */
	rate = cnt * 60 / (CHTSIZE * CHTGRAN);
        if (rate > MAXCPM) {
		if (rate > MAXCPM2) 
			r = -1;		/* drop 100% */
		else 
			r -= (cnt % 2); /* drop 50%  */

		if (t - chBest->ch_LogTime > LOG_PAUSE) {
			char src[16];
			snprintf(src, sizeof(src), "%s",
                       		inet_ntoa(ip->ip_src));
                       	syslog(LOG_ERR,
                       		"%s -> %s %d ppm (limit %d/min)",
				src,
                       		inet_ntoa(ip->ip_dst),
				rate,
                		MAXCPM);
			chBest->ch_LogTime = t;
		}
        } 
        return(r);
}
