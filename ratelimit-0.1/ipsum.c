/*
 * ipsum.c
 *
 * A. Cobbs
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

/*
 * SumWords()
 *
 * Do the one's complement sum thing over a range of words
 * Ideally, this should get replaced by an assembly version.
 */

static u_int32_t
/* static inline u_int32_t */
SumWords(u_int16_t *buf, int nwords)
{
  register u_int32_t	sum = 0;

  while (nwords >= 16)
  {
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    sum += (u_int16_t) ntohs(*buf++);
    nwords -= 16;
  }
  while (nwords--)
    sum += (u_int16_t) ntohs(*buf++);
  return(sum);
}

/*
 * IpChecksum()
 *
 * Recompute an IP header checksum
 */

void
IpChecksum(struct ip *ip)
{
  register u_int32_t	sum;

/* Sum up IP header words */

  ip->ip_sum = 0;
  sum = SumWords((u_int16_t *) ip, ip->ip_hl << 1);

/* Flip it & stick it */

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  sum = ~sum;

  ip->ip_sum = htons(sum);
}

/*
 * UdpChecksum()
 *
 * Recompute a UDP checksum on a packet
 *
 * UDP pseudo-header (prot = IPPROTO_UDP = 17):
 *
 *  | source IP address	       |
 *  | dest.  IP address	       |
 *  | zero | prot | UDP leng   |
 *
 * You must set udp->uh_sum = 0 for this to work correctly!
 * Or, if there's already a checksum there, then this will
 * set it to zero iff the checksum is correct.
 */

void
UdpChecksum(struct ip *ip)
{
  struct udphdr	*const udp = (struct udphdr *) ((long *) ip + ip->ip_hl);
  u_int32_t	sum;

/* Do pseudo-header first */

  sum = SumWords((u_int16_t *) &ip->ip_src, 4);
  sum += (u_int16_t) IPPROTO_UDP;
  sum += (u_int16_t) ntohs(udp->uh_ulen);

/* Now do UDP packet itself */

  sum += SumWords((u_int16_t *) udp,
	  ((u_int16_t) ntohs(udp->uh_ulen)) >> 1);
  if (ntohs(udp->uh_ulen) & 1)
    sum += (u_int16_t) (((u_char *) udp)[ntohs(udp->uh_ulen) - 1] << 8);

/* Flip it & stick it */

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  sum = ~sum;

  udp->uh_sum = htons(sum);
}

/*
 * TcpChecksum()
 *
 * Recompute a TCP checksum on a packet
 *
 * TCP pseudo-header (prot = IPPROTO_TCP):
 *
 *  |	source IP address      |
 *  |	dest.  IP address      |
 *  | zero | prot | TCP leng   |
 *
 * You must set tcp->th_sum = 0 for this to work correctly!
 * Or, if there's already a checksum there, then this will
 * set it to zero iff the checksum is correct.
 */

void
TcpChecksum(struct ip *ip)
{
  struct tcphdr	*const tcp = (struct tcphdr *) ((long *) ip + ip->ip_hl);
  u_int32_t	sum;
  int		tcp_len;

/* Calculate total length of the TCP segment */

  tcp_len = (u_int16_t) ntohs(ip->ip_len) - (ip->ip_hl << 2);

/* Do pseudo-header first */

  sum = SumWords((u_int16_t *) &ip->ip_src, 4);
  sum += (u_int16_t) IPPROTO_TCP;
  sum += (u_int16_t) tcp_len;

/* Sum up tcp part */

  sum += SumWords((u_int16_t *) tcp, tcp_len >> 1);
  if (tcp_len & 1)
    sum += (u_int16_t) (((u_char *) tcp)[tcp_len - 1] << 8);

/* Flip it & stick it */

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  sum = ~sum;

  tcp->th_sum = htons(sum);
}

/*
 * IcmpChecksum()
 *
 * Recompute an ICMP checksum on a packet
 */

void
IcmpChecksum(struct ip *ip)
{
  struct icmp	*const icmp = (struct icmp *) ((u_int32_t *) ip + ip->ip_hl);
  int		icmp_len = ntohs(ip->ip_len) - ((char *) icmp - (char *) ip);
  u_int32_t	sum;

/* Sum up ICMP packet */

  icmp->icmp_cksum = 0;
  sum = SumWords((u_int16_t *) icmp, icmp_len >> 1);
  if (icmp_len & 1)
    sum += (u_int16_t) (((u_char *) icmp)[icmp_len - 1] << 8);

/* Flip it & stick it */

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  sum = ~sum;

  icmp->icmp_cksum = htons(sum);
}

#if MAIN
/*
 * main()
 */

static u_char pingdata[] =
{
 0x45, 0x00, 0x00, 0x54, 0x98, 0xf9, 0x00, 0x00,
 0xff, 0x01, 0x1c, 0xaa, 0x01, 0x01, 0x01, 0x01,
 0x02, 0x02, 0x02, 0x02, 0x08, 0x00, 0xbd, 0x9f,
 0xaa, 0x14, 0x00, 0x00, 0xde, 0x0e, 0xa6, 0x32,
 0x19, 0x07, 0x08, 0x00, 0x08, 0x09, 0x0a, 0x0b,
 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33,
};

void
main(void)
{
  struct ip	*ip = (struct ip *) pingdata;

  ip->ip_sum = 0;
  IpChecksum(ip);
  printf("The checksum of the packet is 0x%02x%02x\n",
    ((u_char *) &ip->ip_sum)[0], ((u_char *) &ip->ip_sum)[1]);
}
#endif
