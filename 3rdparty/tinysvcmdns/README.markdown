tinysvcmdns
============
tinysvcmdns is a tiny MDNS responder implementation for publishing services.

This implementation is only concerned with publishing services, without a 
system-wide daemon like Bonjour or Avahi. Its other goal is to be extremely 
small, embeddable, and have no external dependencies.

It only answers queries related to its own hostname (the A record), the 
service PTRs, and the "_services.dns-sd._udp.local" name, which advertises 
all services on a particular host.

Services consist of a single SRV and TXT record.

Decoding of MDNS packets is only done to retrieve the questions and answer RRs.
The purpose for decoding answer RRs is to make sure the service PTR is not 
sent out if it is already included in the answer RRs.

It also only utilizes multicast packets, so no "QU" queries are accepted.

There is no name collision detection, so this means no queries are generated
before publishing the services. However compliant responders will avoid using 
our names, since the implementation will respond to queries that match our 
name.


TODO
-----
 * better, more stable & complete API
 * name collision detection


FILES
------
 * mdns.c - provides data structures, parsing & encoding of MDNS packets
 * mdnsd.c - implements the server socket, communication and thread
 * testmdnsd.c - an example that creates an instance until terminated


LICENSE
--------
tinysvcmdns is licensed under the 3-clause ("modified") BSD License.

Copyright (C) 2011 Darell Tan

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

