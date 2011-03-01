/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef STANZAEXTENSION_H__
#define STANZAEXTENSION_H__

#include "macros.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * Supported Stanza extension types.
   */
  enum StanzaExtensionType
  {
    ExtNone,                        /**< Invalid StanzaExtension. */
    ExtVCardUpdate,                 /**< Extension in the vcard-temp:x:update namespace,
                                     * advertising a user avatar's SHA1 hash (XEP-0153). */
    ExtOOB,                         /**< An extension in the jabber:iq:oob or jabber:x:oob
                                     * namespaces (XEP-0066). */
    ExtGPGSigned,                   /**< An extension containing a GPG/PGP signature
                                     * (XEP-0027). */
    ExtGPGEncrypted,                /**< An extension containing a GPG/PGP encrypted message
                                     * (XEP-0027). */
    ExtReceipt,                     /**< An extension containing a Message Receipt/Request
                                     * (XEP-0184). */
    ExtDelay,                       /**< An extension containing notice of delayed delivery
                                     * (XEP-0203 & XEP-0091). */
    ExtAMP,                         /**< An extension containing advanced message processing
                                     * rules (XEP-0079). */
    ExtError,                       /**< An extension containing an error. */
    ExtCaps,                        /**< An extension containing Entity Capabilities
                                     * (XEP-0115). */
    ExtChatState,                   /**< An extension containing a chat state (XEP-0085). */
    ExtMessageEvent,                /**< An extension containing a message event (XEP-0022). */
    ExtDataForm,                    /**< An extension containing a Data Form (XEP-0004). */
    ExtNickname,                    /**< An extension containing a User Nickname (XEP-0172). */
    ExtResourceBind,                /**< A resource bind SE (RFC3921). */
    ExtSessionCreation,             /**< A session establishing SE (RFC3921). */
    ExtVersion,                     /**< An extension containing a Version request/reply
                                     * (XEP-0092). */
    ExtXHtmlIM,                     /**< An extension containing an XHTML message
                                     * representation (XEP-0071) */
    ExtDiscoInfo,                   /**< An extension containing a disco#info element (XEP-0030). */
    ExtDiscoItems,                  /**< An extension containing a disco#items element (XEP-0030). */
    ExtAdhocCommand,                /**< An extension containing a Adhoc Command (XEP-0050). */
    ExtPrivateXML,                  /**< An extension used for Private XML Storage (XEP-0048). */
    ExtRoster,                      /**< An extension dealing with the user's roster (RFC-3921). */
    ExtFeatureNeg,                  /**< An extension abstracting a Feature Negotiation element
                                     * (XEP-0020). */
    ExtIBB,                         /**< An extension dealing with IBBs (XEP-0047). */
    ExtNonSaslAuth,                 /**< An extension for doing Non-SASL Authentication (XEP-0078). */
    ExtMUC,                         /**< An extension dealing with the muc namespace of XEP-0045. */
    ExtMUCOwner,                    /**< An extension dealing with the muc#owner namespace of XEP-0045. */
    ExtMUCAdmin,                    /**< An extension dealing with the muc#admin namespace of XEP-0045. */
    ExtMUCUser,                     /**< An extension dealing with the muc#user namespace of XEP-0045. */
    ExtMUCUnique,                   /**< An extension dealing with the muc#unique namespace of XEP-0045. */
    ExtPing,                        /**< An XMPP Ping (XEP-0199). */
    ExtSearch,                      /**< A XEP-0055 (Jabber Search) wrapper. */
    ExtRegistration,                /**< A XEP-0077 (In-Band Registration) wrapper. */
    ExtJingle,                      /**< An extension dealing with Jingle (XEP-0166) */
    ExtVCard,                       /**< An extension dealing with vcard-temp (XEP-0054) */
    ExtPrivacy,                     /**< An extension dealing with Privacy Lists (XEP-0016) */
    ExtLastActivity,                /**< An extension dealing with Last Activity (XEP-0012).  */
    ExtFlexOffline,                 /**< An extension dealing with Flexible Offline Messages (XEP-0013). */
    ExtSI,                          /**< An extension dealing with Stream Initiation (XEP-0095). */
    ExtS5BQuery,                    /**< An extension dealing with stream host offers (XEP-0065) */
    ExtPubSub,                      /**< An extension dealing with PubSub requests (XEP-0060). */
    ExtPubSubOwner,                 /**< An extension dealing with PubSub owner requests (XEP-0060). */
    ExtPubSubEvent,                 /**< An extension for PubSub event notifications
                                     * (XEP-0060) */
    ExtSHIM,                        /**< An extension dealing with Stanza Headers and Internet Metadata (XEP-0131). */
    ExtAttention,                   /**< An extension dealing with Attention (XEP-0224). */
    ExtUser                         /**< User-supplied extensions must use IDs above this. Do
                                     * not hard-code ExtUser's value anywhere, it is subject
                                     * to change. */
  };

  /**
   * @brief This class abstracts a stanza extension, which is usually
   * an element in a specific namespace.
   *
   * This class is the base class for almost all protocol extensions in gloox.
   * As such, it should be used whenever an add-on to the core XMPP spec
   * needs to be made. For simple protocols it may suffice to create a sub-class
   * of StanzaExtension. For protocols which require keeping of state, an additional
   * persistent object acting like a manager may be needed.
   *
   * A Stanza can be extended by additional namespaced child elements. Obviously,
   * it is not viable to include all the kinds of extensions possible. To avoid
   * hard-coding of such extensions into gloox, StanzaExtension can be used to
   * inform the core of gloox about additional supported extensions without it
   * needing to know about the exact implementation.
   *
   * Note that a StanzaExtension can be used for both sending and receiving
   * of custom protocols. When receiving, gloox requires an appropriate implementation
   * of the pure virtuals filterString() and newInstance(). To be able to properly use
   * the encapsulation, some getters may be necessary. Note that the object you will be
   * dealing with usually is @em const.
   * For sending StanzaExtensions, a custom constructor (as well as some setters,
   * possibly) is needed. Additionally, an implementation of tag() is required.
   *
   * @li Sub-class StanzaExtension and re-implement filterString(). filterString()
   * is supposed to return an XPath expression that matches the child element
   * of a stanza that the protocol-to-implement uses. For example, consider this
   * hypothetical XML format: The protocol is encapsulated inside a &lt;stats&gt;
   * element in the 'ext:stats' namespace. It uses IQ stanzas for transmission.
   * @code
   * <iq from='...' to='...' id='id1' type='get'>
   *   <stats xmlns='ext:stats'/>
   * </iq>
   *
   * <iq from='...' to='...' id='id1' type='result'>
   *   <stats xmlns='ext:stats'>
   *     <users>10</users>
   *   </stats>
   * </iq>
   * @endcode
   * The idea of filterString() and its XPath expression is to match the
   * &lt;stats&gt; element such that it can be fed to your
   * StanzaExtension-derived class' constructor when creating a new instance
   * of it. For our @e stats protocol, filterString() would return something like:
   * <em>/iq/stats[\@xmlns='ext:stats']</em>
   *
   * @li When subclassing StanzaExtension, you have to initialize it with an int, the extension's
   * type. You should choose a value that is not yet used in gloox, and unique to
   * the given extension you implement. In general, you are free to use values
   * above @link gloox::ExtUser ExtUser @endlink. See
   * @link gloox::StanzaExtensionType StanzaExtensionType @endlink for existing values.
   *
   * @li The next step is to implement newInstance(). Whenever filterString()'s
   * XPath expression matches a child element of an incoming stanza, newInstance()
   * is called with the matched Tag. For our example above, this is the &lt;stats&gt;
   * element (including its children):
   * @code
   * <stats xmlns='ext:stats'>
   *   <users>10</users>
   * </stats>
   * @endcode
   * The purpose of newInstance() is to return a new instance of your specialized
   * StanzaExtension (implicitly cast to StanzaExtension). This way, gloox can deal
   * entirely with the abstract base, StanzaExtension, and never ever needs to know
   * which kind of extension it deals with. The most common implementation of
   * newInstance() looks like this:
   * @code
   * StanzaExtension* StatsExtension::newInstance( const Tag* tag ) const
   * {
   *   return new StatsExtension( tag );
   * }
   * @endcode
   * This of course implies that a constructor exists that takes a <em>const Tag*</em> as the
   * only parameter.
   *
   * @li Finally, gloox must be able to serialize the StanzaExtension back
   * into string'ified XML. This is done by means of the tag() function which
   * must be reimplemented. The output Tag should -- like the input Tag -- be embeddable
   * into the respective stanza.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API StanzaExtension
  {
    public:
      /**
       * Constructs an empty StanzaExtension.
       * @param type Designates the extension's type. It should be one of StanzaExtensionType
       * for built-in extensions, and it should be higher than ExtUser for custom types.
       */
      StanzaExtension( int type ) : m_valid( false ), m_extensionType( type ) {}

      /**
       * Virtual destructor.
       */
      virtual ~StanzaExtension() {}

      /**
       * Returns an XPath expression that describes a path to child elements of a
       * stanza that an extension handles.
       *
       * @return The extension's filter string.
       */
      virtual const std::string& filterString() const = 0;

      /**
       * Returns a new Instance of the derived type. Usually, for a derived class FooExtension,
       * the implementation of this function looks like:
       * @code
       * StanzaExtension* FooExtension::newInstance( const Tag* tag ) const
       * {
       *   return new FooExtension( tag );
       * }
       * @endcode
       * @return The derived extension's new instance.
       */
      virtual StanzaExtension* newInstance( const Tag* tag ) const = 0;

      /**
       * Returns a Tag representation of the extension.
       * @return A Tag representation of the extension.
       */
      virtual Tag* tag() const = 0;

      /**
       * Returns an identical copy of the current StanzaExtension.
       * @return An identical copy of the current StanzaExtension.
       */
      virtual StanzaExtension* clone() const = 0;

      /**
       * Returns the extension's type.
       * @return The extension's type.
       */
      int extensionType() const { return m_extensionType; }

    protected:
      bool m_valid;

    private:
      int m_extensionType;

  };

}

#endif // STANZAEXTENSION_H__
