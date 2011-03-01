/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef MUCROOMHANDLER_H__
#define MUCROOMHANDLER_H__

#include "gloox.h"
#include "presence.h"
#include "disco.h"

#include <string>

namespace gloox
{

  class JID;
  class MUCRoom;
  class Message;
  class DataForm;

  /**
   * Describes a participant in a MUC room.
   */
  struct MUCRoomParticipant
  {
    JID* nick;                      /**< Pointer to a JID holding the participant's full JID
                                     * in the form @c room\@service/nick. <br>
                                     * @note The MUC server @b may change the chosen nickname.
                                     * If the @b self member of this struct is true, one should
                                     * check the resource of this member if the actual nickname
                                     * is important. */
    MUCRoomAffiliation affiliation; /**< The participant's affiliation with the room. */
    MUCRoomRole role;               /**< The participant's role with the room. */
    JID* jid;                       /**< Pointer to the occupant's full JID in a non-anonymous room or
                                     * in a semi-anonymous room if the user (of gloox) has a role of
                                     * moderator.
                                     * 0 if the MUC service doesn't provide the JID. */
    int flags;                      /**< ORed MUCUserFlag values. Indicate conditions like: user has
                                     * been kicked or banned from the room. Also may indicate that
                                     * this struct refers to this instance's user.
                                     * (MUC servers send presence to all room occupants, including
                                     * the originator of the presence.) */
    std::string reason;             /**< If the presence change is the result of an action where the
                                     * actor can provide a reason for the action, this reason is stored
                                     * here. Examples: Kicking, banning, leaving the room. */
    JID* actor;                     /**< If the presence change is the result of an action of a room
                                     * member, a pointer to the actor's JID is stored here, if the
                                     * actor chose to disclose his or her identity. Examples: Kicking
                                     * and banning.
                                     * 0 if the identity is not disclosed. */
    std::string newNick;            /**< In case of a nickname change, this holds the new nick, while the
                                     * nick member holds the old room nick (in JID form). @c newNick is only
                                     * set if @c flags contains @b UserNickChanged. If @c flags contains
                                     * @b UserSelf as well, a foregoing nick change request (using
                                     * MUCRoom::setNick()) can be considered acknowledged. In any case
                                     * the user's presence sent with the nick change acknowledgement
                                     * is of type @c unavailable. Another presence of type @c available
                                     * (or whatever the user's presence was at the time of the nick change
                                     * request) will follow (not necessarily immediately) coming from the
                                     * user's new nickname. Empty if there is no nick change in progress. */
    std::string status;             /**< If the presence packet contained a status message, it is stored
                                     * here. */
    JID* alternate;                 /**< If @c flags contains UserRoomDestroyed, and if the user who
                                     * destroyed the room specified an alternate room, this member holds
                                     * a pointer to the alternate room's JID, else it is 0. */
  };

  /**
   * @brief This interface enables inheriting classes to be notified about certain events in a MUC room.
   *
   * See MUCRoom for examples how to use this interface.
   *
   * @note This interface does not notify about room configuration related events. Use
   * MUCRoomConfigHandler for that puprose.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API MUCRoomHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~MUCRoomHandler() {}

      /**
       * This function is called whenever a room occupant enters the room, changes presence
       * inside the room, or leaves the room.
       * @note The MUCRoomParticipant struct, including pointers to JIDs, will be cleaned up after
       * this function returned.
       * @param room The room.
       * @param participant A struct describing the occupant's status and/or action.
       * @param presence The occupant's full presence.
       */
      virtual void handleMUCParticipantPresence( MUCRoom* room, const MUCRoomParticipant participant,
          const Presence& presence ) = 0;

      /**
       * This function is called when a message arrives through the room.
       * @note This may be a private message! If the message is private, and you want to answer
       * it privately, you should create a new MessageSession to the user's full room nick and use
       * that for any further private communication with the user.
       * @param room The room the message came from.
       * @param msg The entire Message.
       * @param priv Indicates whether this is a private message.
       * @note The sender's nick name can be obtained with this call:
       * @code
       * const std::string nick = msg.from().resource();
       * @endcode
       * @note The message may contain an extension of type DelayedDelivery describing the
       * date/time when the message was originally sent. The presence of such an extension
       * usually indicates that the message is sent as part of the room history. This extension
       * can be obtained with this call:
       * @code
       * const DelayedDelivery* dd = msg.when(); // may be 0 if no such extension exists
       * @endcode
       */
      virtual void handleMUCMessage( MUCRoom* room, const Message& msg, bool priv ) = 0;

      /**
       * This function is called if the room that was just joined didn't exist prior to the attempted
       * join. Therfore the room was created by MUC service. To accept the default configuration of
       * the room assigned by the MUC service, return @b true from this function. The room will be opened
       * by the MUC service and available for other users to join. If you don't want to accept the default
       * room configuration, return @b false from this function. The room will stay locked until it is
       * either fully configured, created as an instant room, or creation is canceled.
       *
       * If you returned false from this function you should use one of the following options:
       * @li use MUCRoom::cancelRoomCreation() to abort creation and delete the room,
       * @li use MUCRoom::acknowledgeInstantRoom() to accept the room's default configuration, or
       * @li use MUCRoom::requestRoomConfig() to request the room's configuration form.
       *
       * @param room The room.
       * @return @b True to accept the default room configuration, @b false to keep the room locked
       * until configured manually by the room owner.
       */
      virtual bool handleMUCRoomCreation( MUCRoom* room ) = 0;

      /**
       * This function is called when the room subject has been changed.
       * @param room The room.
       * @param nick The nick of the occupant that changed the room subject.
       * @note With some MUC services the nick may be empty when a room is first entered.
       * @param subject The new room subject.
       */
      virtual void handleMUCSubject( MUCRoom* room, const std::string& nick,
                                     const std::string& subject ) = 0;

      /**
       * This function is called when the user invited somebody (e.g., by using MUCRoom::invite())
       * to the room, but the invitation was declined by that person.
       * @param room The room.
       * @param invitee The JID if the person that declined the invitation.
       * @param reason An optional  reason for declining the invitation.
       */
      virtual void handleMUCInviteDecline( MUCRoom* room, const JID& invitee,
                                           const std::string& reason ) = 0;

      /**
       * This function is called when an error occurs in the room or when entering the room.
       * @note The following error conditions are specified for MUC:
       * @li @b Not @b Authorized: Password required.
       * @li @b Forbidden: Access denied, user is banned.
       * @li @b Item @b Not @b Found: The room does not exist.
       * @li @b Not @b Allowed: Room creation is restricted.
       * @li @b Not @b Acceptable: Room nicks are locked down.
       * @li @b Registration @b Required: User is not on the member list.
       * @li @b Conflict: Desired room nickname is in use or registered by another user.
       * @li @b Service @b Unavailable: Maximum number of users has been reached.
       *
       * Other errors might appear, depending on the service implementation.
       * @param room The room.
       * @param error The error.
       */
      virtual void handleMUCError( MUCRoom* room, StanzaError error ) = 0;

      /**
       * This function usually (see below) is called in response to a call to MUCRoom::getRoomInfo().
       * @param room The room.
       * @param features ORed MUCRoomFlag's.
       * @param name The room's name as returned by Service Discovery.
       * @param infoForm A DataForm containing extended room information. May be 0 if the service
       * doesn't support extended room information. See Section 15.5 of XEP-0045 for defined
       * field types. You should not delete the form.
       *
       * @note This function may be called without a prior call to MUCRoom::getRoomInfo(). This
       * happens if the room config is changed, e.g. by a room admin.
       */
      virtual void handleMUCInfo( MUCRoom* room, int features, const std::string& name,
                                  const DataForm* infoForm ) = 0;

      /**
       * This function is called in response to a call to MUCRoom::getRoomItems().
       * @param room The room.
       * @param items A map of room participants. The key is the name, the value is the occupant's
       * room JID. The map may be empty if such info is private.
       */
      virtual void handleMUCItems( MUCRoom* room, const Disco::ItemList& items ) = 0;

  };

}

#endif// MUCROOMHANDLER_H__
