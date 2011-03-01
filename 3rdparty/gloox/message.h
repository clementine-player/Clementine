/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef MESSAGE_H__
#define MESSAGE_H__

#include "delayeddelivery.h"
#include "stanza.h"

#include <string>

namespace gloox
{

  class JID;

  /**
   * @brief An abstraction of a message stanza.
   *
   * @author Vincent Thomasset
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class GLOOX_API Message : public Stanza
  {

    friend class ClientBase;

    public:

      /**
       * Describes the different valid message types.
       */
      enum MessageType
      {
        Chat           =  1,        /**< A chat message. */
        Error          =  2,        /**< An error message. */
        Groupchat      =  4,        /**< A groupchat message. */
        Headline       =  8,        /**< A headline message. */
        Normal         = 16,        /**< A normal message. */
        Invalid        = 32         /**< The message stanza is invalid. */
      };

      /**
       * Creates a Message.
       * @param type The message type.
       * @param to The intended receiver.
       * @param body The message's body text.
       * @param subject The message's optional subject.
       * @param thread The message's optional thread ID.
       * @param xmllang An optional xml:lang for the message body.
       */
      Message( MessageType type, const JID& to,
               const std::string& body = EmptyString, const std::string& subject = EmptyString,
               const std::string& thread = EmptyString, const std::string& xmllang = EmptyString );

      /**
       * Destructor.
       */
      virtual ~Message();

      /**
       * Returns the message's type.
       * @return The message's type.
       */
      MessageType subtype() const { return m_subtype; }

      /**
       * Returns the message body for the given language if available.
       * If the requested language is not available, the default body (without a xml:lang
       * attribute) will be returned.
       * @param lang The language identifier for the desired language. It must conform to
       * section 2.12 of the XML specification and RFC 3066. If empty, the default body
       * will be returned, if any.
       * @return The message body.
       */
      const std::string body( const std::string& lang = "default" ) const
      {
        return findLang( m_bodies, m_body, lang );
      }

      /**
       * Returns the message subject for the given language if available.
       * If the requested language is not available, the default subject (without a xml:lang
       * attribute) will be returned.
       * @param lang The language identifier for the desired language. It must conform to
       * section 2.12 of the XML specification and RFC 3066. If empty, the default subject
       * will be returned, if any.
       * @return The message subject.
       */
      const std::string subject( const std::string& lang = "default" ) const
      {
        return findLang( m_subjects, m_subject, lang );
      }

      /**
       * Returns the thread ID of a message stanza.
       * @return The thread ID of a message stanza. Empty for non-message stanzas.
       */
      const std::string& thread() const { return m_thread; }

      /**
       * Sets the thread ID.
       * @param thread The thread ID.
       */
      void setThread( const std::string& thread ) { m_thread = thread; }

      /**
       * Sets the message's ID. Optional.
       * @param id The ID.
       */
      void setID( const std::string& id ) { m_id = id; }

      /**
       * Convenience function that returns a pointer to a DelayedDelivery StanzaExtension, if the
       * message contains one.
       * @return A pointer to a DelayedDelivery object, or 0.
       */
      const DelayedDelivery* when() const
      {
        return static_cast<const DelayedDelivery*>( findExtension( ExtDelay ) );
      }

      // reimplemented from Stanza
      virtual Tag* tag() const;

    private:
#ifdef MESSAGE_TEST
    public:
#endif
      /**
       * Creates a message Stanza from the given Tag. The original Tag will be ripped off.
       * @param tag The Tag to parse.
       */
      Message( Tag* tag );

      MessageType m_subtype;
      std::string m_body;
      std::string m_subject;
      StringMap* m_bodies;
      StringMap* m_subjects;
      std::string m_thread;
  };

}

#endif // MESSAGE_H__
