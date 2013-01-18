/*
 * tinysvcmdns - a tiny MDNS implementation for publishing services
 * Copyright (C) 2011 Darell Tan
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef _WIN32
#include <winsock2.h>
#include <in6addr.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include "mdns.h"
#include "mdnsd.h"

int main(int argc, char *argv[]) {
	// create host entries
	char *hostname = "some-random-host.local";

	struct mdnsd *svr = mdnsd_start();
	if (svr == NULL) {
		printf("mdnsd_start() error\n");
		return 1;
	}

	printf("mdnsd_start OK. press ENTER to add hostname & service\n");
	getchar();

	mdnsd_set_hostname(svr, hostname, inet_addr("192.168.0.29"));

	struct rr_entry *a2_e = NULL;
	a2_e = rr_create_a(create_nlabel(hostname), inet_addr("192.168.0.31"));
	mdnsd_add_rr(svr, a2_e);

	struct rr_entry *aaaa_e = NULL;

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_NUMERICHOST;
	struct addrinfo* results;
	getaddrinfo(
		"fe80::e2f8:47ff:fe20:28e0",
		NULL,
		&hints,
		&results);
	struct sockaddr_in6* addr = (struct sockaddr_in6*)results->ai_addr;
	struct in6_addr v6addr = addr->sin6_addr;
	freeaddrinfo(results);

	aaaa_e = rr_create_aaaa(create_nlabel(hostname), &v6addr);

	mdnsd_add_rr(svr, aaaa_e);

	const char *txt[] = {
		"path=/mywebsite", 
		NULL
	};
	struct mdns_service *svc = mdnsd_register_svc(svr, "My Website", 
									"_http._tcp.local", 8080, NULL, txt);
	mdns_service_destroy(svc);

	printf("added service and hostname. press ENTER to exit\n");
	getchar();

	mdnsd_stop(svr);

	return 0;
}

