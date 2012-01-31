/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


/*
 * udpred - a inetd launched udp port redirector
 *
 * Version:	@(#)udpred.c 0.01   01/15/97
 *
 * Author:	Chris Faherty <chrisf@america.com>
 *
 * syntax:
 *
 * udpred toip toport
 *
 * sample inetd.conf entry:
 *
 * 7000	dgram	udp	wait	root	/usr/sbin/tcpd	/usr/sbin/udpred 192.168.100.16 7000
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <syslog.h>

#endif

int host_port; // port we are listening on

typedef struct peer {
	time_t last;
	struct sockaddr_in sin;
	struct sockaddr_in dest;
	int s; // connected socket to remote
	struct peer *next;
} peer_t;

peer_t *peers;

/*
====================
NET_Init
====================
*/
void NET_Init (void)
{
#ifdef _WIN32
	static WSADATA          winsockdata;
//    WORD    wVersionRequested;
    int             r;

//    wVersionRequested = MAKEWORD(1, 1);

    r = WSAStartup (MAKEWORD(2, 1), &winsockdata);

    if (r) {
		fprintf(stderr, "Winsock initialization failed.");
		exit(1);
	}

    printf("Winsock Initialized\n");
#endif
}



int connectsock(char *host, char *service, char *protocol)
{
	struct hostent *phe;
	struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in sin;
	int s, type;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

/* Map service name to port number */
	if(pse = getservbyname(service, protocol))
		sin.sin_port = pse->s_port;
	else if((sin.sin_port = htons((u_short)atoi(service))) == 0)
	{
		fprintf(stderr, "udpred: can't get \"%s\" service entry\n", service);
		exit(2);
	}

/* Map host name to IP address, allowing for dotted decimal */
	if(phe = gethostbyname(host))
		memcpy((char *)&sin.sin_addr, phe->h_addr, phe->h_length);
	else if((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
	{
		fprintf(stderr, "udpred: can't get \"%s\" host entry\n", host);
		exit(2);
	}

/* Map protocol name to protocol number */
	if((ppe = getprotobyname(protocol)) == 0)
	{
		fprintf(stderr, "udpred: can't get \"%s\" protocol entry\n", protocol);
		exit(2);
	}

/* Use protocol to choose a socket type */
	if(strcmp(protocol, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

/* Allocate a socket */
	s = socket(PF_INET, type, ppe->p_proto);
	if(s < 0)
	{
		fprintf(stderr, "udpred: can't create socket: %s\n", sys_errlist[errno]);
		exit(2);
	}

/* Connect the socket */
	if(connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		fprintf(stderr, "udpred: can't connect to %s.%s: %s\n", host, service, sys_errlist[errno]);
		exit(2);
	}
	return s;
}

int main(int argc, char *argv[])
{
	fd_set rfds;
	struct timeval tv;
	int retval;
	int i1;
	char buffer[4095];
	struct sockaddr_in fsin;
	int alen;
	peer_t *p;
	int s;
	struct sockaddr_in	address;

	if (argc < 3) {
		printf("Usage:  %s <port> <remote server> <remote server port>\n", argv[0]);
		return 1;
	}

	NET_Init();

	if ((s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return 1;
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons((unsigned short)atoi(argv[1]));
	if (bind (s, (void *)&address, sizeof(address)) == -1)
		perror("bind");

	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(s, &rfds);
		i1 = s;
		for (p = peers; p; p = p->next) {
			FD_SET(p->s, &rfds);
			if (p->s >= i1)
				i1 = p->s + 1;
		}
		/* Wait up to two minutes. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		retval = select(i1, &rfds, (fd_set *)0, (fd_set *)0, &tv);
		if(retval > 0)
		{
			if(FD_ISSET(s, &rfds))
			{
				alen = sizeof(fsin);
				i1 = recvfrom(s, buffer, 4096, 0, (struct sockaddr *) &fsin, &alen);
				if(i1 > 0) {
					for (p = peers; p; p = p->next)
						if (memcmp(&p->sin.sin_addr, &fsin.sin_addr, sizeof(p->sin.sin_addr)) == 0 &&
							memcmp(&p->sin.sin_port, &fsin.sin_port, sizeof(p->sin.sin_port)) == 0) 
						{
							send(p->s, buffer, i1, 0);
							time(&p->last);
							break;
						}
					if (p == NULL) { // new peer
						printf("peer %s:%d added", inet_ntoa(fsin.sin_addr), (int)ntohs(fsin.sin_port));
						p = malloc(sizeof *p);
						p->sin = fsin;
						p->s = connectsock(argv[2], argv[3], "udp");
						p->next = peers;
						peers = p;
						send(p->s, buffer, i1, 0);
						time(&p->last);
					}
				}
			}
			for (p = peers; p; p = p->next)
				if(FD_ISSET(p->s, &rfds))
				{
					i1 = recv(p->s, buffer, 4096, 0);
					if(i1 > 0) {
						time(&p->last);
						sendto(s, buffer, i1, 0, (struct sockaddr *) &p->sin, 
							sizeof(p->sin)); 
					}
				}
		} else {
			peer_t *pp;

			pp = NULL;
			p = peers; 
			while (p) {
				if (time(NULL) - p->last > 300) {
					if (!pp && !p->next) {
						printf("peer %s:%d removed (timeout)", inet_ntoa(p->sin.sin_addr), (int)ntohs(p->sin.sin_port));
						free(p);
						p = pp = NULL;
						continue;
					}
					pp->next = p->next;
					printf ("peer %s:%d removed (timeout)", inet_ntoa(p->sin.sin_addr), (int)ntohs(p->sin.sin_port));
					free(p);
					p = pp->next;
				} else {
					pp = p;
					p = p->next;
				}
			}
		}
	}
}
