/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef JID_H__
#define JID_H__

#include "macros.h"

#include <string>

namespace gloox
{
  /**
   * @brief An abstraction of a JID.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.4
   */
  class GLOOX_API JID
  {
    public:

      /**
       * Constructs an empty JID.
       */
      JID() : m_valid( false ) {}

      /**
       * Constructs a new JID from a string.
       * @param jid The string containing the JID.
       */
      JID( const std::string& jid ) : m_valid( true ) { setJID( jid ); }

      /**
       * Destructor.
       */
      ~JID() {}

      /**
       * Sets the JID from a string.
       * @param jid The string containing the JID.
       * @return @b True if the given JID was valid, @b false otherwise.
       */
      bool setJID( const std::string& jid );

      /**
       * Returns the full (prepped) JID (user\@host/resource).
       * @return The full JID.
       */
      const std::string& full() const { return m_full; }

      /**
       * Returns the bare (prepped) JID (user\@host).
       * @return The bare JID.
       */
      const std::string& bare() const { return m_bare; }

      /**
       * Creates and returns a JID from this JID's node and server parts.
       * @return The bare JID.
       * @since 0.9
       */
      JID bareJID() const { return JID( bare() ); }

      /**
       * Sets the username.
       * @param username The new username.
       */
      bool setUsername( const std::string& username );

      /**
       * Sets the server.
       * @param server The new server.
       */
      bool setServer( const std::string& server );

      /**
       * Sets the resource.
       * @param resource The new resource.
       */
      bool setResource( const std::string& resource );

      /**
       * Returns the prepped username.
       * @return The current username.
       */
      const std::string& username() const { return m_username; }

      /**
       * Returns the prepped server name.
       * @return The current server.
       */
      const std::string& server() const { return m_server; }

      /**
       * Returns the raw (unprepped) server name.
       * @return The raw server name.
       */
      const std::string& serverRaw() const { return m_serverRaw; }

      /**
       * Returns the prepped resource.
       * @return The current resource.
       */
      const std::string& resource() const { return m_resource; }

      /**
       * Compares a JID with a string.
       * @param right The second JID in string representation.
       */
      bool operator==( const std::string& right ) const { return full() == right; }

      /**
       * Compares a JID with a string.
       * @param right The second JID in string representation.
       */
      bool operator!=( const std::string& right ) const { return full() != right; }

      /**
       * Compares two JIDs.
       * @param right The second JID.
       */
      bool operator==( const JID& right ) const { return full() == right.full(); }

      /**
       * Compares two JIDs.
       * @param right The second JID.
       */
      bool operator!=( const JID& right ) const { return full() != right.full(); }

      /**
       * Converts to  @b true if the JID is valid, @b false otherwise.
       */
      operator bool() const { return m_valid; }

      /**
       * XEP-0106: JID Escaping
       * @param node The node to escape.
       * @return The escaped node.
       */
      static std::string escapeNode( const std::string& node );

      /**
       * XEP-0106: JID Escaping
       * @param node The node to unescape.
       * @return The unescaped node.
       */
      static std::string unescapeNode( const std::string& node );

    private:
      /**
       * Utility function to rebuild both the bare and full jid.
       */
      void setStrings() { setBare(); setFull(); }

      /**
       * Utility function rebuilding the bare jid.
       * @note Do not use this function directly, instead use setStrings.
       */
      void setBare();

      /**
       * Utility function rebuilding the full jid.
       */
      void setFull();

      std::string m_resource;
      std::string m_username;
      std::string m_server;
      std::string m_serverRaw;
      std::string m_bare;
      std::string m_full;
      bool m_valid;

  };

}

#endif // JID_H__
