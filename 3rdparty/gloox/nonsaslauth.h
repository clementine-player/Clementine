/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef NONSASLAUTH_H__
#define NONSASLAUTH_H__

#include "iqhandler.h"

#include <string>

namespace gloox
{

  class Client;
  class Stanza;
  class Tag;

  /**
   * @brief This class is an implementation of XEP-0078 (Non-SASL Authentication).
   *
   * It is invoked by @ref Client automatically if supported by the server and if SASL authentication
   * is not supported.
   * You should not need to use this class manually.
   *
   * XEP Version: 2.3
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API NonSaslAuth : public IqHandler
  {
    public:
      /**
       * Constructor.
       * @param parent The @ref ClientBase which is used to authenticate.
       */
      NonSaslAuth( Client* parent );

      /**
       * Virtual Destructor.
       */
      virtual ~NonSaslAuth();

      /**
       * Starts authentication by querying the server for the required authentication fields.
       * Digest authentication is preferred over plain text passwords.
       * @param sid The session ID given by the server with the stream opening tag.
       */
      void doAuth( const std::string& sid );

      // reimplemented from IqHandler
      virtual bool handleIq( const IQ& iq ) { (void)iq; return false; }

      // reimplemented from IqHandler
      virtual void handleIqID( const IQ& iq, int context );

    private:
#ifdef NONSASLAUTH_TEST
    public:
#endif
      /**
       * @brief An abstraction of an IQ extension used for Non-SASL authentication (XEP-0078).
       *
       * @author Jakob Schroeter <js@camaya.net>
       * @since 1.0
       */
      class Query : public StanzaExtension
      {
        public:
          /**
           * Creates a new object that can be used to query the server for
           * authentication filds for the given user.
           * @param user The user name to fetch authentication fields for.
           */
          Query( const std::string& user );

          /**
           * Creates a now object from the given Tag.
           * @param tag The Tag to parse.
           */
          Query( const Tag* tag = 0 );

          /**
           * Creates a new object on the heap that can be used to
           * authenticate, based on the current reply.
           * @param user The uset o authenticate as.
           * @param sid The stream's ID.
           * @param pwd The password to use.
           * @param resource The desired resource identifier.
           */
          Query* newInstance( const std::string& user, const std::string& sid,
                              const std::string& pwd, const std::string& resource ) const;

          /**
           * Virtual destructor.
           */
          virtual ~Query() {}

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new Query( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            return new Query( *this );
          }

        private:
          std::string m_user;
          std::string m_pwd;
          std::string m_resource;
          bool m_digest;

      };

      enum NonSaslAuthTrack
      {
        TrackRequestAuthFields,
        TrackSendAuth
      };

      Client* m_parent;
      std::string m_sid;

  };

}

#endif // NONSASLAUTH_H__
