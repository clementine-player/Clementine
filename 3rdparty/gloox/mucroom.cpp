/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "mucroom.h"
#include "clientbase.h"
#include "dataform.h"
#include "presence.h"
#include "disco.h"
#include "mucmessagesession.h"
#include "message.h"
#include "error.h"
#include "util.h"
#include "tag.h"

namespace gloox
{

  // ---- MUCRoom::MUCAdmin ----
  /* Error type values */
  static const char* affiliationValues [] = {
    "none",
    "outcast",
    "member",
    "owner",
    "admin"
  };

  /* Stanza error values */
  static const char* roleValues [] = {
    "none",
    "visitor",
    "participant",
    "moderator",
  };

  /** Strings indicating the type of history to request. */
  const char* historyTypeValues[] =
  {
    "maxchars", "maxstanzas", "seconds", "since"
  };

  static inline MUCRoomAffiliation affiliationType( const std::string& type )
  {
    return (MUCRoomAffiliation)util::lookup( type, affiliationValues );
  }

  static inline MUCRoomRole roleType( const std::string& type )
  {
    return (MUCRoomRole)util::lookup( type, roleValues );
  }

  MUCRoom::MUCAdmin::MUCAdmin( MUCRoomRole role, const std::string& nick,
                               const std::string& reason )
    : StanzaExtension( ExtMUCAdmin ), m_affiliation( AffiliationInvalid ), m_role( role )
  {
    m_list.push_back( MUCListItem( nick, role, reason ) );
  }

  MUCRoom::MUCAdmin::MUCAdmin( MUCRoomAffiliation affiliation, const std::string& nick,
                               const std::string& reason )
    : StanzaExtension( ExtMUCAdmin ), m_affiliation( affiliation ), m_role( RoleInvalid )
  {
    m_list.push_back( MUCListItem( nick, affiliation, reason ) );
  }

  MUCRoom::MUCAdmin::MUCAdmin( MUCOperation operation, const MUCListItemList& jids )
    : StanzaExtension( ExtMUCAdmin ), m_list( jids ), m_affiliation( AffiliationInvalid ),
      m_role( RoleInvalid )
  {
    switch( operation )
    {
      case StoreVoiceList:
      case RequestVoiceList:
        m_role = RoleParticipant;
        break;
      case StoreModeratorList:
      case RequestModeratorList:
        m_role = RoleModerator;
        break;
      case StoreBanList:
      case RequestBanList:
        m_affiliation = AffiliationOutcast;
        break;
      case StoreMemberList:
      case RequestMemberList:
        m_affiliation = AffiliationMember;
        break;
      case StoreOwnerList:
      case RequestOwnerList:
        m_affiliation = AffiliationOwner;
        break;
      case StoreAdminList:
      case RequestAdminList:
        m_affiliation = AffiliationAdmin;
        break;
      default:
        return;
        break;
    }

    if( m_list.empty() )
      m_list.push_back( MUCListItem( JID() ) );
  }

  MUCRoom::MUCAdmin::MUCAdmin( const Tag* tag )
    : StanzaExtension( ExtMUCAdmin ), m_affiliation( AffiliationInvalid ), m_role( RoleInvalid )
  {
    if( !tag || tag->name() != "query" || tag->xmlns() != XMLNS_MUC_ADMIN )
      return;

    const TagList& items = tag->findChildren( "item" );
    TagList::const_iterator it = items.begin();
    for( ; it != items.end(); ++it )
    {
      m_list.push_back( MUCListItem( JID( (*it)->findAttribute( "jid" ) ),
                        roleType( (*it)->findAttribute( "role" ) ),
                        affiliationType( (*it)->findAttribute( "affiliation" ) ),
                        (*it)->findAttribute( "nick" ) ) );
      if( m_role == RoleInvalid )
        m_role = roleType( (*it)->findAttribute( "role" ) );
      if( m_affiliation == AffiliationInvalid )
        m_affiliation = affiliationType( (*it)->findAttribute( "affiliation" ) );
    }
  }

  MUCRoom::MUCAdmin::~MUCAdmin()
  {
  }

  const std::string& MUCRoom::MUCAdmin::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_MUC_ADMIN + "']";
    return filter;
  }

  Tag* MUCRoom::MUCAdmin::tag() const
  {
    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_MUC_ADMIN );

    if( m_list.empty() || ( m_affiliation == AffiliationInvalid && m_role == RoleInvalid ) )
      return t;

    MUCListItemList::const_iterator it = m_list.begin();
    for( ; it != m_list.end(); ++it )
    {
      Tag* i = new Tag( t, "item" );
      if( (*it).jid() )
        i->addAttribute( "jid", (*it).jid().bare() );
      if( !(*it).nick().empty() )
        i->addAttribute( "nick", (*it).nick() );

      MUCRoomRole rol = RoleInvalid;
      if( (*it).role() != RoleInvalid )
        rol = (*it).role();
      else if( m_role != RoleInvalid )
        rol = m_role;
      if( rol != RoleInvalid )
        i->addAttribute( "role", util::lookup( rol, roleValues ) );

      MUCRoomAffiliation aff = AffiliationInvalid;
      if( (*it).affiliation() != AffiliationInvalid )
        aff = (*it).affiliation();
      else if( m_affiliation != AffiliationInvalid )
        aff = m_affiliation;
      if( aff != AffiliationInvalid )
        i->addAttribute( "affiliation", util::lookup( aff, affiliationValues ) );
      if( !(*it).reason().empty() )
        new Tag( i, "reason", (*it).reason() );
    }

    return t;
  }
  // ---- ~MUCRoom::MUCAdmin ----

  // ---- MUCRoom::MUCOwner ----
  MUCRoom::MUCOwner::MUCOwner( QueryType type, DataForm* form )
    : StanzaExtension( ExtMUCOwner ), m_type( type ), m_form( form )
  {
    m_valid = true;

    if( m_form )
      return;

    switch( type )
    {
      case TypeCancelConfig:
        m_form = new DataForm( TypeCancel );
        break;
      case TypeInstantRoom:
        m_form = new DataForm( TypeSubmit );
        break;
      default:
        break;
    }
  }

  MUCRoom::MUCOwner::MUCOwner( const JID& alternate, const std::string& reason,
                               const std::string& password )
    : StanzaExtension( ExtMUCOwner ), m_type( TypeDestroy ), m_jid( alternate ),
      m_reason( reason ), m_pwd( password ), m_form( 0 )
  {
    m_valid = true;
  }

  MUCRoom::MUCOwner::MUCOwner( const Tag* tag )
    : StanzaExtension( ExtMUCOwner ), m_type( TypeIncomingTag ), m_form( 0 )
  {
    if( !tag || tag->name() != "query" || tag->xmlns() != XMLNS_MUC_OWNER )
      return;

    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      const std::string& name = (*it)->name();
      if( name == "x" && (*it)->xmlns() == XMLNS_X_DATA )
      {
        m_form = new DataForm( (*it) );
        break;
      }
      else if( name == "destroy" )
      {
        m_type = TypeDestroy;
        m_jid = (*it)->findAttribute( "jid" );
        m_pwd = (*it)->findCData( "/query/destroy/password" );
        m_reason = (*it)->findCData( "/query/destroy/reason" );
        break;
      }
    }
    m_valid = true;
  }

  MUCRoom::MUCOwner::~MUCOwner()
  {
    delete m_form;
  }

  const std::string& MUCRoom::MUCOwner::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_MUC_OWNER + "']";
    return filter;
  }

  Tag* MUCRoom::MUCOwner::tag() const
  {
    if( !m_valid )
      return 0;

    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_MUC_OWNER );

    switch( m_type )
    {
      case TypeInstantRoom:
      case TypeSendConfig:
      case TypeCancelConfig:
      case TypeIncomingTag:
        if( m_form )
          t->addChild( m_form->tag() );
        break;
      case TypeDestroy:
      {
        Tag* d = new Tag( t, "destroy" );
        if( m_jid )
          d->addAttribute( "jid", m_jid.bare() );

        if( !m_reason.empty() )
          new Tag( d, "reason", m_reason );

        if( !m_pwd.empty() )
          new Tag( d, "password", m_pwd );

        break;
      }
      case TypeRequestConfig:
      case TypeCreate:
      default:
        break;
    }

    return t;
  }
  // ---- ~MUCRoom::MUCOwner ----

  // ---- MUCRoom::MUCUser ----
  MUCRoom::MUCUser::MUCUser( MUCUserOperation operation, const std::string& to,
                             const std::string& reason, const std::string& thread )
    : StanzaExtension( ExtMUCUser ), m_affiliation( AffiliationInvalid ), m_role( RoleInvalid ),
      m_jid( new std::string( to ) ), m_actor( 0 ),
      m_thread( thread.empty() ? 0 : new std::string( thread ) ),
      m_reason( new std::string( reason ) ), m_newNick( 0 ), m_password( 0 ), m_alternate( 0 ),
      m_operation( operation ),
      m_flags( 0 ), m_del( false ), m_continue( !thread.empty() )
  {
  }

  MUCRoom::MUCUser::MUCUser( const Tag* tag )
    : StanzaExtension( ExtMUCUser ), m_affiliation( AffiliationInvalid ), m_role( RoleInvalid ),
      m_jid( 0 ), m_actor( 0 ), m_thread( 0 ), m_reason( 0 ), m_newNick( 0 ),
      m_password( 0 ), m_alternate( 0 ), m_operation( OpNone ),
      m_flags( 0 ), m_del( false ), m_continue( false )
  {
    if( !tag || tag->name() != "x" || tag->xmlns() != XMLNS_MUC_USER )
      return;

    const Tag* t = 0;
    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "item" )
      {
        m_affiliation = getEnumAffiliation( (*it)->findAttribute( "affiliation" ) );
        m_role = getEnumRole( (*it)->findAttribute( "role" ) );

        if( (*it)->hasAttribute( "jid" ) )
          m_jid = new std::string( (*it)->findAttribute( "jid" ) );

        if( ( t = (*it)->findChild( "actor" ) ) )
          m_actor = new std::string( t->findAttribute( "jid" ) );

        if( ( t = (*it)->findChild( "reason" ) ) )
          m_reason = new std::string( t->cdata() );

        if( (*it)->hasAttribute( "nick" ) )
          m_newNick = new std::string( (*it)->findAttribute( "nick" ) );
      }
      else if( (*it)->name() == "status" )
      {
        const std::string& code = (*it)->findAttribute( "code" );
        if( code == "100" )
          m_flags |= FlagNonAnonymous;
        else if( code == "101" )
          m_flags |= UserAffiliationChangedWNR;
        else if( code == "110" )
          m_flags |= UserSelf;
        else if( code == "170" )
          m_flags |= FlagPublicLogging;
        else if( code == "201" )
          m_flags |= UserNewRoom;
        else if( code == "210" )
          m_flags |= UserNickAssigned;
        else if( code == "301" )
          m_flags |= UserBanned;
        else if( code == "303" )
          m_flags |= UserNickChanged;
        else if( code == "307" )
          m_flags |= UserKicked;
        else if( code == "321" )
          m_flags |= UserAffiliationChanged;
        else if( code == "322" )
          m_flags |= UserMembershipRequired;
        else if( code == "332" )
          m_flags |= UserRoomShutdown;
      }
      else if( (*it)->name() == "destroy" )
      {
        m_del = true;
        if( (*it)->hasAttribute( "jid" ) )
          m_alternate = new std::string( (*it)->findAttribute( "jid" ) );

        if( ( t = (*it)->findChild( "reason" ) ) )
          m_reason = new std::string( t->cdata() );

        m_flags  |= UserRoomDestroyed;
      }
      else if( (*it)->name() == "invite" )
      {
        m_operation = OpInviteFrom;
        m_jid = new std::string( (*it)->findAttribute( "from" ) );
        if( m_jid->empty() )
        {
          m_operation = OpInviteTo;
          m_jid->assign( (*it)->findAttribute( "to" ) );
        }
        if( (*it)->hasChild( "reason" ) )
          m_reason = new std::string( (*it)->findChild( "reason" )->cdata() );
        if( (*it)->hasChild( "continue" ) )
        {
          m_continue = true;
          m_thread = new std::string( (*it)->findChild( "continue" )->findAttribute( "thread" ) );
        }
      }
      else if( (*it)->name() == "decline" )
      {
        m_operation = OpDeclineFrom;
        m_jid = new std::string( (*it)->findAttribute( "from" ) );
        if( m_jid->empty() )
        {
          m_operation = OpDeclineTo;
          m_jid->assign( (*it)->findAttribute( "from" ) );
        }
        if( (*it)->hasChild( "reason" ) )
          m_reason = new std::string( (*it)->findChild( "reason" )->cdata() );
      }
      else if( (*it)->name() == "password" )
      {
        m_password = new std::string( (*it)->cdata() );
      }
    }
  }

  MUCRoom::MUCUser::~MUCUser()
  {
    delete m_jid;
    delete m_actor;
    delete m_thread;
    delete m_reason;
    delete m_newNick;
    delete m_password;
    delete m_alternate;
  }

  MUCRoomRole MUCRoom::MUCUser::getEnumRole( const std::string& role )
  {
    if( role == "moderator" )
      return RoleModerator;
    if( role == "participant" )
      return RoleParticipant;
    if( role == "visitor" )
      return RoleVisitor;
    return RoleNone;
  }

  MUCRoomAffiliation MUCRoom::MUCUser::getEnumAffiliation( const std::string& affiliation )
  {
    if( affiliation == "owner" )
      return AffiliationOwner;
    if( affiliation == "admin" )
      return AffiliationAdmin;
    if( affiliation == "member" )
      return AffiliationMember;
    if( affiliation == "outcast" )
      return AffiliationOutcast;
    return AffiliationNone;
  }

  const std::string& MUCRoom::MUCUser::filterString() const
  {
    static const std::string filter = "/presence/x[@xmlns='" + XMLNS_MUC_USER + "']"
                                      "|/message/x[@xmlns='" + XMLNS_MUC_USER + "']";
    return filter;
  }

  Tag* MUCRoom::MUCUser::tag() const
  {
    Tag* t = new Tag( "x" );
    t->setXmlns( XMLNS_MUC_USER );

    if( m_affiliation != AffiliationInvalid || m_role != RoleInvalid )
    {
      Tag* i = new Tag( t, "item" );
      if( m_jid )
        i->addAttribute( "jid", *m_jid );
      if( m_role != RoleInvalid )
        i->addAttribute( "role", util::lookup( m_role, roleValues ) );
      if( m_affiliation != AffiliationInvalid )
        i->addAttribute( "affiliation", util::lookup( m_affiliation, affiliationValues ) );

      if( m_actor )
        new Tag( i, "actor", "jid", *m_actor );

      if( m_flags & FlagNonAnonymous )
        new Tag( t, "status", "code", "100" );
      if( m_flags & UserAffiliationChangedWNR )
        new Tag( t, "status", "code", "101" );
      if( m_flags & UserSelf )
        new Tag( t, "status", "code", "110" );
      if( m_flags & FlagPublicLogging )
        new Tag( t, "status", "code", "170" );
      if( m_flags & UserNewRoom )
        new Tag( t, "status", "code", "201" );
      if( m_flags & UserNickAssigned )
        new Tag( t, "status", "code", "210" );
      if( m_flags & UserBanned )
        new Tag( t, "status", "code", "301" );
      if( m_flags & UserNickChanged )
        new Tag( t, "status", "code", "303" );
      if( m_flags & UserKicked )
        new Tag( t, "status", "code", "307" );
      if( m_flags & UserAffiliationChanged )
        new Tag( t, "status", "code", "321" );
      if( m_flags & UserMembershipRequired )
        new Tag( t, "status", "code", "322" );
      if( m_flags & UserRoomShutdown )
        new Tag( t, "status", "code", "332" );
    }
    else if( m_del )
    {
      Tag* d = new Tag( t, "destroy" );
      if( m_alternate )
        d->addAttribute( "jid", *m_alternate );
      if( m_reason )
        new Tag( d, "reason", *m_reason );
    }
    else if( m_operation != OpNone && m_jid )
    {
      Tag* d = 0;
      if( m_operation == OpInviteTo )
        d = new Tag( t, "invite", "to", *m_jid );
      else if( m_operation == OpInviteFrom )
        d = new Tag( t, "invite", "from", *m_jid );
      else if( m_operation == OpDeclineTo )
        d = new Tag( t, "decline", "to", *m_jid );
      else if( m_operation == OpDeclineFrom )
        d = new Tag( t, "decline", "from", *m_jid );

      if( m_reason )
        new Tag( d, "reason", *m_reason );

      if( m_continue )
      {
        Tag* c = new Tag( d, "continue" );
        if( m_thread )
          c->addAttribute( "thread", *m_thread );
      }

      if( m_password )
        new Tag( t, "password", *m_password );

    }

    return t;
  }
  // ---- ~MUCRoom::MUCUser ----

  // ---- MUCRoom::MUC ----
  MUCRoom::MUC::MUC( const std::string& password,
                             MUCRoom::HistoryRequestType historyType,
                             const std::string& historySince,
                             int historyValue )
    : StanzaExtension( ExtMUC ),
      m_password( password.empty() ? 0 : new std::string( password ) ),
      m_historySince( new std::string( historySince ) ),
      m_historyType( historyType ), m_historyValue( historyValue )
  {
  }

  MUCRoom::MUC::MUC( const Tag* tag )
    : StanzaExtension( ExtMUC ),
      m_password( 0 ), m_historySince( 0 ),
      m_historyType( HistoryUnknown ), m_historyValue( 0 )
  {
    if( !tag || tag->name() != "x" || tag->xmlns() != XMLNS_MUC_USER )
      return;

    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      if( (*it)->name() == "history" )
      {
        if( (*it)->hasAttribute( "seconds" ) )
          m_historyValue = atoi( (*it)->findAttribute( "seconds" ).c_str() );
        else if( (*it)->hasAttribute( "maxstanzas" ) )
          m_historyValue = atoi( (*it)->findAttribute( "maxstanzas" ).c_str() );
        else if( (*it)->hasAttribute( "maxchars" ) )
          m_historyValue = atoi( (*it)->findAttribute( "maxchars" ).c_str() );
        else if( (*it)->hasAttribute( "since" ) )
          m_historySince = new std::string( (*it)->findAttribute( "since" ) );
      }
      else if( (*it)->name() == "password" )
      {
        m_password = new std::string( (*it)->cdata() );
      }
    }
  }

  MUCRoom::MUC::~MUC()
  {
    delete m_password;
    delete m_historySince;
  }

  const std::string& MUCRoom::MUC::filterString() const
  {
    static const std::string filter = "/presence/x[@xmlns='" + XMLNS_MUC + "']";
    return filter;
  }

  Tag* MUCRoom::MUC::tag() const
  {
    Tag* t = new Tag( "x" );
    t->setXmlns( XMLNS_MUC );

    if( m_historyType != HistoryUnknown )
    {
      const std::string& histStr = util::lookup( m_historyType, historyTypeValues );
      Tag* h = new Tag( t, "history" );
      if( m_historyType == HistorySince && m_historySince )
        h->addAttribute( histStr, *m_historySince );
      else
        h->addAttribute( histStr, m_historyValue );
    }

    if( m_password )
      new Tag( t, "password", *m_password );

    return t;
  }
  // ---- ~MUCRoom::MUC ----

  // --- MUCRoom ----
  MUCRoom::MUCRoom( ClientBase* parent, const JID& nick, MUCRoomHandler* mrh,
                    MUCRoomConfigHandler* mrch )
    : m_parent( parent ), m_nick( nick ), m_joined( false ), m_roomHandler( mrh ),
      m_roomConfigHandler( mrch ), m_affiliation( AffiliationNone ), m_role( RoleNone ),
      m_historyType( HistoryUnknown ), m_historyValue( 0 ), m_flags( 0 ),
      m_creationInProgress( false ), m_configChanged( false ),
      m_publishNick( false ), m_publish( false ), m_unique( false )
  {
    if( m_parent )
    {
      m_parent->registerStanzaExtension( new MUCAdmin() );
      m_parent->registerStanzaExtension( new MUCOwner() );
      m_parent->registerStanzaExtension( new MUCUser() );
      m_parent->registerStanzaExtension( new MUC() );
      m_parent->registerStanzaExtension( new DelayedDelivery() );
    }
  }

  MUCRoom::~MUCRoom()
  {
    if( m_joined )
      leave();

    if( m_parent )
    {
      if( m_publish )
        m_parent->disco()->removeNodeHandler( this, XMLNS_MUC_ROOMS );

      m_parent->removeIDHandler( this );
//       m_parent->removeStanzaExtension( ExtMUCAdmin ); // don't remove, other rooms might need it
//       m_parent->removeStanzaExtension( ExtMUCOwner );
      m_parent->removePresenceHandler( m_nick.bareJID(), this );
      m_parent->disco()->removeDiscoHandler( this );
    }
  }

  void MUCRoom::join( Presence::PresenceType type, const std::string& status, int priority )
  {
    if( m_joined || !m_parent )
      return;

    m_parent->registerPresenceHandler( m_nick.bareJID(), this );

    m_session = new MUCMessageSession( m_parent, m_nick.bareJID() );
    m_session->registerMessageHandler( this );

    Presence pres( type, m_nick.full(), status, priority );
    pres.addExtension( new MUC( m_password, m_historyType, m_historySince, m_historyValue ) );
    m_joined = true;
    m_parent->send( pres );
  }

  void MUCRoom::leave( const std::string& msg )
  {
    if( !m_joined )
      return;

    if( m_parent )
    {
      Presence pres( Presence::Unavailable, m_nick.full(), msg );
      m_parent->send( pres );
      m_parent->removePresenceHandler( m_nick.bareJID(), this );
      m_parent->disposeMessageSession( m_session );
    }

    m_session = 0;
    m_joined = false;
  }

  void MUCRoom::destroy( const std::string& reason, const JID& alternate, const std::string& password )
  {
    if( !m_parent )
      return;

    const std::string& id = m_parent->getID();
    IQ iq( IQ::Set, m_nick.bareJID(), id );
    iq.addExtension( new MUCOwner( alternate, reason, password ) );
    m_parent->send( iq, this, DestroyRoom );
  }

  void MUCRoom::send( const std::string& message )
  {
    if( m_session && m_joined )
      m_session->send( message );
  }

  void MUCRoom::setSubject( const std::string& subject )
  {
    if( m_session && m_joined )
      m_session->setSubject( subject );
  }

  void MUCRoom::setNick( const std::string& nick )
  {
    if( m_parent && m_joined )
    {
      m_newNick = nick;

      Presence p( Presence::Available, m_nick.bare() + "/" + m_newNick );
      m_parent->send( p );
    }
    else
      m_nick.setResource( nick );
  }

  void MUCRoom::getRoomInfo()
  {
    if( m_parent )
      m_parent->disco()->getDiscoInfo( m_nick.bare(), EmptyString, this, GetRoomInfo );
  }

  void MUCRoom::getRoomItems()
  {
    if( m_parent )
      m_parent->disco()->getDiscoItems( m_nick.bare(), EmptyString, this, GetRoomItems );
  }

  void MUCRoom::setPresence( Presence::PresenceType presence, const std::string& msg )
  {
    if( m_parent && presence != Presence::Unavailable && m_joined )
    {
      Presence p( presence, m_nick.full(), msg );
      m_parent->send( p );
    }
  }

  void MUCRoom::invite( const JID& invitee, const std::string& reason, const std::string& thread )
  {
    if( !m_parent || !m_joined )
      return;

    Message msg( Message::Normal, m_nick.bareJID() );
    msg.addExtension( new MUCUser( OpInviteTo, invitee.bare(), reason, thread ) );
    m_parent->send( msg );
  }

  Message* MUCRoom::declineInvitation( const JID& room, const JID& invitor, const std::string& reason )
  {
    Message* msg = new Message( Message::Normal, room.bare() );
    msg->addExtension( new MUCUser( OpDeclineTo, invitor.bare(), reason ) );
    return msg;
  }

  void MUCRoom::setPublish( bool publish, bool publishNick )
  {
    m_publish = publish;
    m_publishNick = publishNick;

    if( !m_parent )
      return;

    if( m_publish )
      m_parent->disco()->registerNodeHandler( this, XMLNS_MUC_ROOMS );
    else
      m_parent->disco()->removeNodeHandler( this, XMLNS_MUC_ROOMS );
  }

  void MUCRoom::addHistory( const std::string& message, const JID& from, const std::string& stamp )
  {
    if( !m_joined || !m_parent )
      return;

    Message m( Message::Groupchat, m_nick.bareJID(), message );
    m.addExtension( new DelayedDelivery( from, stamp ) );
    m_parent->send( m );
  }

  void MUCRoom::setRequestHistory( int value, MUCRoom::HistoryRequestType type )
  {
    m_historyType = type;
    m_historySince = EmptyString;
    m_historyValue = value;
  }

  void MUCRoom::setRequestHistory( const std::string& since )
  {
    m_historyType = HistorySince;
    m_historySince = since;
    m_historyValue = 0;
  }

  Message* MUCRoom::createDataForm( const JID& room, const DataForm* df )
  {
    Message* m = new Message( Message::Normal, room.bare() );
    m->addExtension( df );
    return m;
  }

  void MUCRoom::requestVoice()
  {
    if( !m_parent || !m_joined )
      return;

    DataForm* df = new DataForm( TypeSubmit );
    df->addField( DataFormField::TypeNone, "FORM_TYPE", XMLNS_MUC_REQUEST );
    df->addField( DataFormField::TypeTextSingle, "muc#role", "participant", "Requested role" );

    Message m( Message::Normal, m_nick.bare() );
    m.addExtension( df );

    m_parent->send( m );
  }

  void MUCRoom::setRole( const std::string& nick, MUCRoomRole role,
                         const std::string& reason )
  {
    if( !m_parent || !m_joined || nick.empty() || role == RoleInvalid )
      return;

    MUCOperation action = InvalidOperation;
    switch( role )
    {
      case RoleNone:
        action = SetRNone;
        break;
      case RoleVisitor:
        action = SetVisitor;
        break;
      case RoleParticipant:
        action = SetParticipant;
        break;
      case RoleModerator:
        action = SetModerator;
        break;
      default:
        break;
    }

    IQ iq( IQ::Set, m_nick.bareJID() );
    iq.addExtension( new MUCAdmin( role, nick, reason ) );

    m_parent->send( iq, this, action );
  }

  void MUCRoom::setAffiliation( const std::string& nick, MUCRoomAffiliation affiliation,
                                const std::string& reason )
  {
    if( !m_parent || !m_joined || nick.empty() || affiliation == AffiliationInvalid )
      return;

    MUCOperation action = InvalidOperation;
    switch( affiliation )
    {
      case AffiliationOutcast:
        action = SetOutcast;
        break;
      case AffiliationNone:
        action = SetANone;
        break;
      case AffiliationMember:
        action = SetMember;
        break;
      case AffiliationAdmin:
        action = SetAdmin;
        break;
      case AffiliationOwner:
        action = SetOwner;
        break;
      default:
        break;
    }

    IQ iq( IQ::Set, m_nick.bareJID() );
    iq.addExtension( new MUCAdmin( affiliation, nick, reason ) );

    m_parent->send( iq, this, action );
  }

  void MUCRoom::requestList( MUCOperation operation )
  {
    if( !m_parent || !m_joined || !m_roomConfigHandler )
      return;

    IQ iq( IQ::Get, m_nick.bareJID() );
    iq.addExtension( new MUCAdmin( operation ) );
    m_parent->send( iq, this, operation );
  }

  void MUCRoom::storeList( const MUCListItemList items, MUCOperation operation )
  {
    if( !m_parent || !m_joined )
      return;

    IQ iq( IQ::Set, m_nick.bareJID() );
    iq.addExtension( new MUCAdmin( operation , items ) );
    m_parent->send( iq, this, operation );
  }

  void MUCRoom::handlePresence( const Presence& presence )
  {
    if( ( presence.from().bare() != m_nick.bare() ) || !m_roomHandler )
      return;

    if( presence.subtype() == Presence::Error  )
    {
      if( m_newNick.empty() )
      {
        m_parent->removePresenceHandler( m_nick.bareJID(), this );
        m_parent->disposeMessageSession( m_session );
        m_joined = false;
        m_session = 0;
      }
      else
        m_newNick = "";

      m_roomHandler->handleMUCError( this, presence.error()
                                           ? presence.error()->error()
                                           : StanzaErrorUndefined );
    }
    else
    {
      const MUCUser* mu = presence.findExtension<MUCUser>( ExtMUCUser );
      if( !mu )
        return;

      MUCRoomParticipant party;
      party.nick = new JID( presence.from() );
      party.status = presence.status();
      party.affiliation = mu->affiliation();
      party.role = mu->role();
      party.jid = mu->jid() ? new JID( *(mu->jid()) ) : 0;
      party.actor = mu->actor() ? new JID( *(mu->actor()) ) : 0;
      party.reason = mu->reason() ? *(mu->reason()) : EmptyString;
      party.newNick = mu->newNick() ? *(mu->newNick()) : EmptyString;
      party.alternate = mu->alternate() ? new JID( *(mu->alternate()) ) : 0;
      party.flags = mu->flags();

      if( party.flags & FlagNonAnonymous )
        setNonAnonymous();

      if( party.flags & UserSelf )
      {
        m_role = party.role;
        m_affiliation = party.affiliation;
      }
      if( party.flags & UserNewRoom )
      {
        m_creationInProgress = true;
        if( instantRoomHook() || m_roomHandler->handleMUCRoomCreation( this ) )
          acknowledgeInstantRoom();
      }
      if( party.flags & UserNickAssigned )
        m_nick.setResource( presence.from().resource() );

      if( party.flags & UserNickChanged && !party.newNick.empty()
          && m_nick.resource() == presence.from().resource()
          && party.newNick == m_newNick )
        party.flags |= UserSelf;

      if( party.flags & UserNickChanged && party.flags & UserSelf && !party.newNick.empty() )
        m_nick.setResource( party.newNick );

      if( m_roomHandler )
        m_roomHandler->handleMUCParticipantPresence( this, party, presence );

      delete party.nick;
    }
  }

  void MUCRoom::instantRoom( int context )
  {
    if( !m_creationInProgress || !m_parent || !m_joined )
      return;

    IQ iq( IQ::Set, m_nick.bareJID() );
    iq.addExtension( new MUCOwner( context == CreateInstantRoom
                                     ? MUCOwner::TypeInstantRoom : MUCOwner::TypeCancelConfig ) );

    m_parent->send( iq, this, context );

    m_creationInProgress = false;
  }

  void MUCRoom::requestRoomConfig()
  {
    if( !m_parent || !m_joined )
      return;

    IQ iq( IQ::Get, m_nick.bareJID() );
    iq.addExtension( new MUCOwner( MUCOwner::TypeRequestConfig ) );

    m_parent->send( iq, this, RequestRoomConfig );

    if( m_creationInProgress )
      m_creationInProgress = false;
  }

  void MUCRoom::setRoomConfig( DataForm* form )
  {
    if( !m_parent || !m_joined )
      return;

    IQ iq( IQ::Set, m_nick.bareJID() );
    iq.addExtension( new MUCOwner( MUCOwner::TypeSendConfig, form ) );

    m_parent->send( iq, this, SendRoomConfig );
  }

  void MUCRoom::setNonAnonymous()
  {
    m_flags |= FlagNonAnonymous;
    m_flags &= ~( FlagSemiAnonymous | FlagFullyAnonymous );
  }

  void MUCRoom::setSemiAnonymous()
  {
    m_flags &= ~( FlagNonAnonymous | FlagFullyAnonymous );
    m_flags |= FlagSemiAnonymous;
  }

  void MUCRoom::setFullyAnonymous()
  {
    m_flags &= ~( FlagNonAnonymous | FlagSemiAnonymous );
    m_flags |= FlagFullyAnonymous;
  }

  void MUCRoom::handleMessage( const Message& msg, MessageSession* /*session*/ )
  {
    if( !m_roomHandler )
      return;

    if( msg.subtype() == Message::Error )
    {
      m_roomHandler->handleMUCError( this, msg.error() ? msg.error()->error() : StanzaErrorUndefined );
    }
    else
    {
      const MUCUser* mu = msg.findExtension<MUCUser>( ExtMUCUser );
      if( mu )
      {
        const int flags = mu->flags();
        if( flags & FlagNonAnonymous )
          setNonAnonymous();
        if( flags & FlagPublicLogging )
        {
          m_flags &= ~FlagPublicLoggingOff;
          m_flags |= FlagPublicLogging;
        }
        if( flags & FlagPublicLoggingOff )
        {
          m_flags &= ~FlagPublicLogging;
          m_flags |= FlagPublicLoggingOff;
        }
        if( flags & FlagSemiAnonymous )
          setSemiAnonymous();
        if( flags & FlagFullyAnonymous )
          setFullyAnonymous();

        if( mu->operation() == OpDeclineFrom && mu->jid() )
          m_roomHandler->handleMUCInviteDecline( this, JID( *(mu->jid()) ),
                                                 mu->reason() ? *(mu->reason()) : EmptyString );
      }

      const DataForm* df = msg.findExtension<DataForm>( ExtDataForm );
      if( m_roomConfigHandler && df )
      {
        m_roomConfigHandler->handleMUCRequest( this, *df );
        return;
      }

      if( !msg.subject().empty() )
      {
        m_roomHandler->handleMUCSubject( this, msg.from().resource(), msg.subject() );
      }
      else if( !msg.body().empty() )
      {
        std::string when;
        bool privMsg = false;
        bool history = false;
        if( msg.when() )
        {
          when = msg.when()->stamp();
          history = true;
        }
        if( msg.subtype() & ( Message::Chat | Message::Normal ) )
          privMsg = true;

        m_roomHandler->handleMUCMessage( this, msg, privMsg );
      }
    }
  }

  void MUCRoom::handleIqID( const IQ& iq, int context )
  {
    if( !m_roomConfigHandler )
      return;

    switch( iq.subtype() )
    {
      case IQ::Result:
        handleIqResult( iq, context );
        break;
      case IQ::Error:
        handleIqError( iq, context );
        break;
      default:
        break;
    }
  }

  void MUCRoom::handleIqResult( const IQ& iq, int context )
  {
    switch( context )
    {
      case SetRNone:
      case SetVisitor:
      case SetParticipant:
      case SetModerator:
      case SetANone:
      case SetOutcast:
      case SetMember:
      case SetAdmin:
      case SetOwner:
      case CreateInstantRoom:
      case CancelRoomCreation:
      case DestroyRoom:
      case StoreVoiceList:
      case StoreBanList:
      case StoreMemberList:
      case StoreModeratorList:
      case StoreAdminList:
        m_roomConfigHandler->handleMUCConfigResult( this, true, (MUCOperation)context );
        break;
      case RequestRoomConfig:
      {
        const MUCOwner* mo = iq.findExtension<MUCOwner>( ExtMUCOwner );
        if( !mo )
          break;

        if( mo->form() )
          m_roomConfigHandler->handleMUCConfigForm( this, *(mo->form()) );
        break;
      }
      case RequestVoiceList:
      case RequestBanList:
      case RequestMemberList:
      case RequestModeratorList:
      case RequestOwnerList:
      case RequestAdminList:
      {
        const MUCAdmin* ma = iq.findExtension<MUCAdmin>( ExtMUCAdmin );
        if( !ma )
          break;

        m_roomConfigHandler->handleMUCConfigList( this, ma->list(), (MUCOperation)context );
        break;
      }
      default:
        break;
    }
  }

  void MUCRoom::handleIqError( const IQ& /*iq*/, int context )
  {
    switch( context )
    {
      case SetRNone:
      case SetVisitor:
      case SetParticipant:
      case SetModerator:
      case SetANone:
      case SetOutcast:
      case SetMember:
      case SetAdmin:
      case SetOwner:
      case CreateInstantRoom:
      case CancelRoomCreation:
      case RequestRoomConfig:
      case DestroyRoom:
      case RequestVoiceList:
      case StoreVoiceList:
      case RequestBanList:
      case StoreBanList:
      case RequestMemberList:
      case StoreMemberList:
      case RequestModeratorList:
      case StoreModeratorList:
      case RequestOwnerList:
      case StoreOwnerList:
      case RequestAdminList:
      case StoreAdminList:
        m_roomConfigHandler->handleMUCConfigResult( this, false, (MUCOperation)context );
        break;
    }
  }

  void MUCRoom::handleDiscoInfo( const JID& /*from*/, const Disco::Info& info, int context )
  {
    switch( context )
    {
      case GetRoomInfo:
      {
        int oldflags = m_flags;
        m_flags = 0;
        if( oldflags & FlagPublicLogging )
          m_flags |= FlagPublicLogging;

        std::string name;
        const StringList& l = info.features();
        StringList::const_iterator it = l.begin();
        for( ; it != l.end(); ++it )
        {
          if( (*it) == "muc_hidden" )
            m_flags |= FlagHidden;
          else if( (*it) == "muc_membersonly" )
            m_flags |= FlagMembersOnly;
          else if( (*it) == "muc_moderated" )
            m_flags |= FlagModerated;
          else if( (*it) == "muc_nonanonymous" )
            setNonAnonymous();
          else if( (*it) == "muc_open" )
            m_flags |= FlagOpen;
          else if( (*it) == "muc_passwordprotected" )
            m_flags |= FlagPasswordProtected;
          else if( (*it) == "muc_persistent" )
            m_flags |= FlagPersistent;
          else if( (*it) == "muc_public" )
            m_flags |= FlagPublic;
          else if( (*it) == "muc_semianonymous" )
            setSemiAnonymous();
          else if( (*it) == "muc_temporary" )
            m_flags |= FlagTemporary;
          else if( (*it) == "muc_fullyanonymous" )
            setFullyAnonymous();
          else if( (*it) == "muc_unmoderated" )
            m_flags |= FlagUnmoderated;
          else if( (*it) == "muc_unsecured" )
            m_flags |= FlagUnsecured;
        }

        const Disco::IdentityList& il = info.identities();
        if( il.size() )
          name = il.front()->name();

        if( m_roomHandler )
          m_roomHandler->handleMUCInfo( this, m_flags, name, info.form() );
        break;
      }
      default:
        break;
    }
  }

  void MUCRoom::handleDiscoItems( const JID& /*from*/, const Disco::Items& items, int context )
  {
    if( !m_roomHandler )
      return;

    switch( context )
    {
      case GetRoomItems:
      {
        m_roomHandler->handleMUCItems( this, items.items() );
        break;
      }
      default:
        break;
    }
  }

  void MUCRoom::handleDiscoError( const JID& /*from*/, const Error* /*error*/, int context )
  {
    if( !m_roomHandler )
      return;

    switch( context )
    {
      case GetRoomInfo:
        m_roomHandler->handleMUCInfo( this, 0, EmptyString, 0 );
        break;
      case GetRoomItems:
        m_roomHandler->handleMUCItems( this, Disco::ItemList() );
        break;
      default:
        break;
    }
  }

  StringList MUCRoom::handleDiscoNodeFeatures( const JID& /*from*/, const std::string& /*node*/ )
  {
    return StringList();
  }

  Disco::IdentityList MUCRoom::handleDiscoNodeIdentities( const JID& /*from*/,
                                                          const std::string& /*node*/ )
  {
    return Disco::IdentityList();
  }

  Disco::ItemList MUCRoom::handleDiscoNodeItems( const JID& /*from*/, const JID& /*to*/,
                                                 const std::string& node )
  {
    Disco::ItemList l;
    if( node == XMLNS_MUC_ROOMS && m_publish )
    {
      l.push_back( new Disco::Item( m_nick.bareJID(), EmptyString,
                                    m_publishNick ? m_nick.resource() : EmptyString ) );
    }
    return l;
  }

}
