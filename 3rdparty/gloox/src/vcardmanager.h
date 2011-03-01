/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef VCARDMANAGER_H__
#define VCARDMANAGER_H__

#include "gloox.h"
#include "iqhandler.h"

namespace gloox
{

  class ClientBase;
  class VCard;
  class VCardHandler;

  /**
   * @brief A VCardManager can be used to fetch an entities VCard as well as for setting
   * one's own VCard.
   *
   * You need only one VCardManager per Client/ClientBase.
   *
   * @section sec_fetch Fetching a VCard
   *
   * Create a VCardManager and have a VCardHandler ready. Then simply call fetchVCard()
   * and wait for the result.
   * @code
   * class MyClass : public VCardHandler
   * {
   *   public:
   *     MyClass()
   *     {
   *       m_myClass = new MyClass();
   *       m_vcardManager = new VCardManager( m_client );
   *     };
   *
   *   ...
   *
   *     void fetchVCard( const JID& jid )
   *     {
   *       m_vcardManager->fetchVCard( jid, this );
   *     };
   *
   *     virtual void handleVCard( const JID& jid, const VCard* vcard );
   *     {
   *       printf( "received vcard\n" );
   *     };
   *
   *     virtual void handleVCardResult( VCardContext context, const JID& jid,
   *                                     StanzaError se )
   *     {
   *       printf( "vcard operation result received\n" );
   *     };
   *
   *   ...
   *
   *   private:
   *     VCardManager* m_vcardManager;
   * };
   * @endcode
   *
   * @section sec_store Storing one's own VCard
   *
   * @note Some, if not many, servers do not implement support for all the fields specified
   * in XEP-0054. Therefore it is possible that you cannot retrieve fields you stored previously.
   *
   * Similar to the above, you need a VCardManager and a VCardHandler. Then construct
   * your VCard and call storeVCard().
   * @code
   *     void storeMyVCard()
   *     {
   *       VCard* v = new VCard();
   *       v->setFormattedname( "Me" );
   *       v->setNickname( "Myself" );
   *       ...
   *       m_vcardManager->storeVCard( v, this );
   *     };
   * @endcode
   *
   * This implementation supports more than one address, address label, email address and telephone number.
   *
   * @note Currently, this implementation lacks support for the following fields:
   * AGENT, CATEGORIES, SOUND, KEY
   *
   * When cleaning up, delete your VCardManager instance @b before deleting the Client/ClientBase instance.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API VCardManager : public IqHandler
  {
    public:
      /**
       * Constructor.
       * @param parent The ClientBase object to use for communication.
       */
      VCardManager( ClientBase* parent );

      /**
       * Virtual destructor.
       */
      virtual ~VCardManager();

      /**
       * Use this function to fetch the VCard of a remote entity or yourself.
       * The result will be announced by calling handleVCard() the VCardHandler.
       * @param jid The entity's JID. Should be a bare JID unless you want to fetch the VCard of, e.g., a MUC item.
       * @param vch The VCardHandler that will receive the result of the VCard fetch.
       */
      void fetchVCard( const JID& jid, VCardHandler* vch );

      /**
       * Use this function to store or update your own VCard on the server. Remember to
       * always send a full VCard, not a delta of changes.
       * If you, for any reason, pass a foreign VCard to this function, your own will be
       * overwritten.
       * @param vcard Your VCard to store.
       * @param vch The VCardHandler that will receive the result of the VCard store.
       */
      void storeVCard( VCard* vcard, VCardHandler* vch );

      /**
       * Use this function, e.g. from your VCardHandler-derived class's dtor, to cancel any
       * outstanding operations (fetchVCard(), storeVCard()). Calling this function even
       * if no operations are pending is just fine.
       * @param vch The VCardHandler to remove from any queues.
       * @since 0.9
       */
      void cancelVCardOperations( VCardHandler* vch );

      // reimplemented from IqHandler.
      virtual bool handleIq( const IQ& iq ) { (void)iq; return false; }

      // reimplemented from IqHandler.
      virtual void handleIqID( const IQ& iq, int context );

    private:
      typedef std::map<std::string, VCardHandler*> TrackMap;
      ClientBase* m_parent;
      TrackMap m_trackMap;

  };

}

#endif // VCARDMANAGER_H__
