/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef MUCROOMCONFIGHANDLER_H__
#define MUCROOMCONFIGHANDLER_H__

#include "gloox.h"
#include "jid.h"

#include <string>
#include <list>

namespace gloox
{

  class MUCRoom;
  class DataForm;

  /**
   * An item in a list of MUC room users. Lists of these items are
   * used when manipulating the lists of members, admins, owners, etc.
   * of a room.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 1.0
   */
  class MUCListItem
  {
    public:
      /**
       * Constructs a new object using the given JID.
       * @param jid The item's JID.
       */
      MUCListItem( const JID& jid )
        : m_jid( jid ), m_affiliation( AffiliationInvalid ), m_role( RoleInvalid )
      {}

      /**
       * Creates a new object, setting JID, affiliation, role, and nick.
       * @param jid The item's JID.
       * @param role The item's role.
       * @param affiliation The item's affiliation.
       * @param nick The item's nick.
       */
      MUCListItem( const JID& jid, MUCRoomRole role, MUCRoomAffiliation affiliation,
                   const std::string& nick )
        : m_jid( jid ), m_nick( nick ), m_affiliation( affiliation ), m_role( role )
      {}

      /**
       * Creates a new object, using nick, affiliation and a reason.
       * @param nick The item's nick.
       * @param affiliation The item's affiliation.
       * @param reason A reason.
       */
      MUCListItem( const std::string& nick, MUCRoomAffiliation affiliation, const std::string& reason )
        : m_nick( nick ), m_affiliation( affiliation ), m_role( RoleInvalid ),
          m_reason( reason )
      {}

      /**
       * Creates a new object, using nick, role and a reason.
       * @param nick The item's nick.
       * @param role The item's role.
       * @param reason A reason.
       */
      MUCListItem( const std::string& nick, MUCRoomRole role, const std::string& reason )
        : m_nick( nick ), m_affiliation( AffiliationInvalid ), m_role( role ),
          m_reason( reason )
      {}

      /**
       * Destructor. Deletes the @c jid member.
       */
      ~MUCListItem() {}

      /**
       * Returns the item's JID.
       * @return The item's JID.
       */
      const JID& jid() const { return m_jid; }

      /**
       * Returns the item's nick.
       * @return The item's nick.
       */
      const std::string& nick() const { return m_nick; }

      /**
       * Returns the item's affiliation.
       * @return The item's affiliation.
       */
      MUCRoomAffiliation affiliation() const { return m_affiliation; }

      /**
       * Returns the item's role.
       * @return The item's role.
       */
      MUCRoomRole role() const { return m_role; }

      /**
       * Returns the reason.
       * @return The reason.
       */
      const std::string& reason() const { return m_reason; }

    private:
      JID m_jid;                   /**< Pointer to the occupant's JID if available, 0 otherwise. */
      std::string m_nick;           /**< The occupant's nick in the room. */
      MUCRoomAffiliation m_affiliation; /**< The occupant's affiliation. */
      MUCRoomRole m_role;           /**< The occupant's role. */
      std::string m_reason;         /**< Use this only when **setting** the item's role/affiliation to
                                     * specify a reason for the role/affiliation change. This field is
                                     * empty in items fetched from the MUC service. */
  };

  /**
   * A list of MUCListItems.
   */
  typedef std::list<MUCListItem> MUCListItemList;

  /**
   * Available operations on a room.
   */
  enum MUCOperation
  {
    RequestUniqueName,              /**< Request a unique room name. */
    CreateInstantRoom,              /**< Create an instant room. */
    CancelRoomCreation,             /**< Cancel room creation process. */
    RequestRoomConfig,              /**< Request room configuration form. */
    SendRoomConfig,                 /**< Send room configuration */
    DestroyRoom,                    /**< Destroy room. */
    GetRoomInfo,                    /**< Fetch room info. */
    GetRoomItems,                   /**< Fetch room items (e.g., current occupants). */
    SetRNone,                       /**< Set a user's role to None. */
    SetVisitor,                     /**< Set a user's role to Visitor. */
    SetParticipant,                 /**< Set a user's role to Participant. */
    SetModerator,                   /**< Set a user's role to Moderator. */
    SetANone,                       /**< Set a user's affiliation to None. */
    SetOutcast,                     /**< Set a user's affiliation to Outcast. */
    SetMember,                      /**< Set a user's affiliation to Member. */
    SetAdmin,                       /**< Set a user's affiliation to Admin. */
    SetOwner,                       /**< Set a user's affiliation to Owner. */
    RequestVoiceList,               /**< Request the room's Voice List. */
    StoreVoiceList,                 /**< Store the room's Voice List. */
    RequestBanList,                 /**< Request the room's Ban List. */
    StoreBanList,                   /**< Store the room's Ban List. */
    RequestMemberList,              /**< Request the room's Member List. */
    StoreMemberList,                /**< Store the room's Member List. */
    RequestModeratorList,           /**< Request the room's Moderator List. */
    StoreModeratorList,             /**< Store the room's Moderator List. */
    RequestOwnerList,               /**< Request the room's Owner List. */
    StoreOwnerList,                 /**< Store the room's Owner List. */
    RequestAdminList,               /**< Request the room's Admin List. */
    StoreAdminList,                 /**< Store the room's Admin List. */
    InvalidOperation                /**< Invalid operation. */
  };

  /**
   * @brief An abstract interface that can be implemented for MUC room configuration.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API MUCRoomConfigHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~MUCRoomConfigHandler() {}

      /**
       * This function is called in response to MUCRoom::requestList() if the list was
       * fetched successfully.
       * @param room The room for which the list arrived.
       * @param items The requestd list's items.
       * @param operation The type of the list.
       */
      virtual void handleMUCConfigList( MUCRoom* room, const MUCListItemList& items,
                                        MUCOperation operation ) = 0;

      /**
       * This function is called when the room's configuration form arrives. This usually happens
       * after a call to MUCRoom::requestRoomConfig(). Use
       * MUCRoom::setRoomConfig() to send the configuration back to the
       * room.
       * @param room The room for which the config form arrived.
       * @param form The configuration form.
       */
      virtual void handleMUCConfigForm( MUCRoom* room, const DataForm& form ) = 0;

      /**
       * This function is called in response to MUCRoom::kick(), MUCRoom::storeList(),
       * MUCRoom::ban(), and others, to indcate the end of the operation.
       * @param room The room for which the operation ended.
       * @param success Whether or not the operation was successful.
       * @param operation The finished operation.
       */
      virtual void handleMUCConfigResult( MUCRoom* room, bool success, MUCOperation operation ) = 0;

      /**
       * This function is called when a Voice request or a Registration request arrive through
       * the room that need to be approved/rejected by the room admin. Use MUCRoom::createDataForm()
       * to have a Tag created that answers the request.
       * @param room The room the request arrived from.
       * @param form A DataForm containing the request.
       */
      virtual void handleMUCRequest( MUCRoom* room, const DataForm& form ) = 0;

  };

}

#endif // MUCROOMCONFIGHANDLER_H__
