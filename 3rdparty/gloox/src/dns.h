/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef DNS_H__
#define DNS_H__

#include "macros.h"
#include "logsink.h"

#ifdef __MINGW32__
# include <windows.h>
# include <windns.h>
#endif

#ifdef HAVE_ARPA_NAMESER_H
# include <arpa/nameser.h>
#endif

#ifdef __APPLE__
# include <arpa/nameser_compat.h>
#endif

#ifndef NS_MAXDNAME
# define NS_MAXDNAME 1025
#endif

#ifndef NS_PACKETSZ
# define NS_PACKETSZ 512
#endif

#ifdef HAVE_GETADDRINFO
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
#endif

#include <string>
#include <map>

namespace gloox
{

  /**
   * @brief This class holds a number of static functions used for DNS related stuff.
   *
   * You should not need to use these functions directly.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API DNS
  {
    public:

      /**
       * A list of strings (used for server addresses) and ints (used for port numbers).
       */
      typedef std::map<std::string, int> HostMap;

      /**
       * This function resolves a service/protocol/domain tuple.
       * @param service The SRV service type.
       * @param proto The SRV protocol.
       * @param domain The domain to search for SRV records.
       * @param logInstance A LogSink to use for logging.
       * @return A list of weighted hostname/port pairs from SRV records, or A records if no SRV
       * records where found.
       */
      static HostMap resolve( const std::string& service, const std::string& proto,
                              const std::string& domain, const LogSink& logInstance );

      /**
       * This is a convenience funtion which uses @ref resolve() to resolve SRV records
       * for a given domain, using a service of @b xmpp-client and a proto of @b tcp.
       * @param domain The domain to resolve SRV records for.
       * @param logInstance A LogSink to use for logging.
       * @return A list of weighted hostname/port pairs from SRV records, or A records if no SRV
       * records where found.
       */
      static HostMap resolve( const std::string& domain, const LogSink& logInstance )
        { return resolve( "xmpp-client", "tcp", domain, logInstance ); }

      /**
       * This is a convenience function which uses @ref resolve() to get a list of hosts
       * and connects to one of them.
       * @param host The host to resolve SRV records for.
       * @param logInstance A LogSink to use for logging.
       * @return A file descriptor for the established connection.
       */
      static int connect( const std::string& host, const LogSink& logInstance );

      /**
       * This is a convenience function which connects to the given host and port. No SRV
       * records are resolved. Use this function for special setups.
       * @param host The host/IP address to connect to.
       * @param port A custom port to connect to.
       * @param logInstance A LogSink to use for logging.
       * @return A file descriptor for the established connection.
       */
      static int connect( const std::string& host, int port, const LogSink& logInstance );

      /**
       * A convenience function that prepares and returnes a simple, unconnected TCP socket.
       * @param logInstance A LogSink to use for logging.
       * @return A TCP socket.
       */
      static int getSocket( const LogSink& logInstance );

      /**
       * Closes the given socket.
       * @param fd The socket to close.
       * @param logInstance A LogSink to use for logging.
       */
      static void closeSocket( int fd, const LogSink& logInstance );

    private:
#ifdef HAVE_GETADDRINFO
      /**
       * Resolves the given service for the given domain and protocol, using the IPv6-ready
       * getaddrinfo(). The result is put into the first parameter.
       * @param res A pointer to a pointer holding the query results.
       * @param service A service string to query for, e.g. xmpp-client.
       * @param proto A protocol name.
       * @param domain The domain to query for.
       * @param logInstance A LogSink to use for logging.
       */
      static void resolve( struct addrinfo** res, const std::string& service, const std::string& proto,
                           const std::string& domain, const LogSink& logInstance );

      /**
       * This is a convenience funtion which uses @ref resolve() to resolve SRV records
       * for a given domain, using a service of @b xmpp-client and a proto of @b tcp.
       * @param res A pointer to a pointer holding the query results.
       * @param domain The domain to resolve SRV records for.
       * @param logInstance A LogSink to use for logging.
       */
      static void resolve( struct addrinfo** res, const std::string& domain, const LogSink& logInstance )
        { resolve( res, "xmpp-client", "tcp", domain, logInstance ); }

      /**
       * Tries to connect to the host/address contained in the addrinfo structure.
       * @param res The connection parameters.
       * @param logInstance A LogSink to use for logging.
       * @return A file descriptor for the established connection.
       */
      static int connect( struct addrinfo* res, const LogSink& logInstance );
#endif

      /**
       * This function prepares and returns a socket with the given parameters.
       * @param af The address family. E.g. PF_INET.
       * @param socktype The socket type. E.g. SOCK_STREAM.
       * @param proto The protocol number. E.g. 6 (TCP).
       */
      static int getSocket( int af, int socktype, int proto, const LogSink& logInstance );

      static HostMap defaultHostMap( const std::string& domain, const LogSink& logInstance );
      static void cleanup( const LogSink& logInstance );

      struct buffer
      {
        unsigned char buf[NS_PACKETSZ];
        int len;
      };
  };

}

#endif // DNS_H__
