/* This file is part of Clementine.
   Copyright 2026, John Maguire <john.maguire@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NETWORKREMOTE_PROTOCOLSNIFFER_H_
#define NETWORKREMOTE_PROTOCOLSNIFFER_H_

// Tells the protocols sharing the network remote's port apart by the first
// byte a client sends. See docs/design/remote-streaming.md section 5.0.
class ProtocolSniffer {
 public:
  enum Protocol {
    // Not a protocol we speak.
    Unknown,
    // The length-prefixed protobuf remote protocol.
    Remote,
    // HTTP/1.x.
    Http,
  };

  // The remote protocol starts with a big-endian length that RemoteClient
  // rejects above 128 MiB, so its first byte is 0x00-0x08. Every HTTP method
  // starts with an upper-case letter. The ranges don't overlap.
  static Protocol Classify(unsigned char first_byte) {
    if (first_byte <= 0x08) return Remote;
    if (first_byte >= 'A' && first_byte <= 'Z') return Http;
    return Unknown;
  }
};

#endif  // NETWORKREMOTE_PROTOCOLSNIFFER_H_
