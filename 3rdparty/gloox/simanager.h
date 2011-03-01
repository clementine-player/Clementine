/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef SIMANAGER_H__
#define SIMANAGER_H__

#include "iqhandler.h"

namespace gloox
{

  class ClientBase;
  class SIProfileHandler;
  class SIHandler;

  /**
   * @brief This class manages streams initiated using XEP-0095.
   *
   * You need only one SIManager object per ClientBase instance.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API SIManager : public IqHandler
  {

    public:
      /**
       * SI error conditions.
       */
      enum SIError
      {
        NoValidStreams,             /**< None of the stream types are acceptable */
        BadProfile,                 /**< Profile is not understood. */
        RequestRejected             /**< SI request was rejected. */
      };

      class SI : public StanzaExtension
      {
        public:
          /**
           * Constructs a new SI object from the given Tag.
           * @param tag The Tag to parse.
           */
          SI( const Tag* tag = 0 );

          /**
           * Constructs a new SI object, wrapping the given Tags.
           * @param tag1 Tag 1.
           * @param tag2 Tag 2.
           */
          SI( Tag* tag1, Tag* tag2, const std::string& id = EmptyString,
              const std::string& mimetype = EmptyString,
              const std::string& profile = EmptyString );

          /**
           * Virtual destructor.
           */
          virtual ~SI();

          /**
           * Returns the current profile namespace.
           * @return The profile namespace.
           */
          const std::string& profile() const { return m_profile; };

          /**
           * Returns the mime-type.
           * @return The mime-type.
           */
          const std::string& mimetype() const { return m_mimetype; };

           /**
           * Returns the SI's ID.
           * @return The SI's id.
            */
          const std::string& id() const { return m_id; };

         /**
           * Returns the first SI child tag.
           * @return The first SI child tag.
           * @todo Use real objects.
           */
          const Tag* tag1() const { return m_tag1; };

          /**
           * Returns the second SI child tag.
           * @return The second SI child tag.
           * @todo Use real objects.
           */
          const Tag* tag2() const { return m_tag2; };

          // reimplemented from StanzaExtension
          virtual const std::string& filterString() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* newInstance( const Tag* tag ) const
          {
            return new SI( tag );
          }

          // reimplemented from StanzaExtension
          virtual Tag* tag() const;

          // reimplemented from StanzaExtension
          virtual StanzaExtension* clone() const
          {
            SI* s = new SI();
            s->m_tag1 = m_tag1 ? m_tag1->clone() : 0;
            s->m_tag2 = m_tag2 ? m_tag2->clone() : 0;
            s->m_id = m_id;
            s->m_mimetype = m_mimetype;
            s->m_profile = m_profile;
            return s;
          }

        private:
          Tag* m_tag1;
          Tag* m_tag2;
          std::string m_id;
          std::string m_mimetype;
          std::string m_profile;
      };

      /**
       * Constructor.
       * @param parent The ClientBase to use for communication.
       * @param advertise Whether to advertise SI capabilities by disco. Defaults to true.
       */
      SIManager( ClientBase* parent, bool advertise = true );

      /**
       * Virtual destructor.
       */
      virtual ~SIManager();

      /**
       * Starts negotiating a stream with a remote entity.
       * @param sih The SIHandler to handle the result of this request.
       * @param to The entity to talk to.
       * @param profile The SI profile to use. See XEP-0095 for more info.
       * @param child1 The first of the two allowed children of the SI offer. See
       * XEP-0095 for more info.
       * @param child2 The second of the two allowed children of the SI offer. See
       * XEP-0095 for more info. Defaults to 0.
       * @param mimetype The stream's/file's mime-type. Defaults to 'binary/octet-stream'.
       * @param from An optional 'from' address to stamp outgoing requests with.
       * Used in component scenario only. Defaults to empty JID.
       * @param sid Optionally specify a stream ID (SID). If empty, one will be generated.
       * @return The requested stream's ID (SID). Empty if SIHandler or ClientBase are invalid.
       * @note The SIManager claims ownership of the Tags supplied to this function, and will
       * delete them after use.
       */
      const std::string requestSI( SIHandler* sih, const JID& to, const std::string& profile, Tag* child1,
                                   Tag* child2 = 0, const std::string& mimetype = "binary/octet-stream",
                                   const JID& from = JID(), const std::string& sid = EmptyString );

      /**
       * Call this function to accept an SI request previously announced by means of
       * SIProfileHandler::handleSIRequest().
       * @param to The requestor.
       * @param id The request's id, as passed to SIProfileHandler::handleSIRequest().
       * @param child1 The &lt;feature/&gt; child of the SI request. See XEP-0095 for details.
       * @param child2 The profile-specific child of the SI request. May be 0. See XEP-0095
       * for details.
       * @param from An optional 'from' address to stamp outgoing stanzas with.
       * Used in component scenario only. Defaults to empty JID.
       * @note The SIManager claims ownership of the Tags supplied to this function, and will
       * delete them after use.
       */
      void acceptSI( const JID& to, const std::string& id, Tag* child1, Tag* child2 = 0, const JID& from = JID() );

      /**
       * Call this function to decline an SI request previously announced by means of
       * SIProfileHandler::handleSIRequest().
       * @param to The requestor.
       * @param id The request's id, as passed to SIProfileHandler::handleSIRequest().
       * @param reason The reason for the reject.
       * @param text An optional human-readable text explaining the decline.
       */
      void declineSI( const JID& to, const std::string& id, SIError reason,
                      const std::string& text = EmptyString );

      /**
       * Registers the given SIProfileHandler to handle requests for the
       * given SI profile namespace. The profile will be advertised by disco (unless disabled in
       * the ctor).
       * @param profile The complete profile namespace, e.g.
       * http://jabber.org/protocol/si/profile/file-transfer.
       * @param sih The profile handler.
       */
      void registerProfile( const std::string& profile, SIProfileHandler* sih );

      /**
       * Un-registers the given profile.
       * @param profile The profile's namespace to un-register.
       */
      void removeProfile( const std::string& profile );

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq );

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    private:
#ifdef SIMANAGER_TEST
    public:
#endif
      enum TrackContext
      {
        OfferSI
      };

      struct TrackStruct
      {
        std::string sid;
        std::string profile;
        SIHandler* sih;
      };
      typedef std::map<std::string, TrackStruct> TrackMap;
      TrackMap m_track;

      ClientBase* m_parent;

      typedef std::map<std::string, SIProfileHandler*> HandlerMap;
      HandlerMap m_handlers;

      bool m_advertise;

  };

}

#endif // SIMANAGER_H__
