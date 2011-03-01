/*
  Copyright (c) 2005-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "registration.h"

#include "clientbase.h"
#include "stanza.h"
#include "error.h"
#include "prep.h"
#include "oob.h"

namespace gloox
{

  // Registration::Query ----
  Registration::Query::Query( DataForm* form )
    : StanzaExtension( ExtRegistration ), m_form( form ), m_fields( 0 ), m_oob( 0 ),
      m_del( false ), m_reg( false )
  {
  }

  Registration::Query::Query( bool del )
    : StanzaExtension( ExtRegistration ), m_form( 0 ), m_fields( 0 ), m_oob( 0 ), m_del( del ),
      m_reg( false )
  {
  }

  Registration::Query::Query( int fields, const RegistrationFields& values )
    : StanzaExtension( ExtRegistration ), m_form( 0 ), m_fields( fields ), m_values( values ),
      m_oob( 0 ), m_del( false ), m_reg( false )
  {
  }

  Registration::Query::Query( const Tag* tag )
    : StanzaExtension( ExtRegistration ), m_form( 0 ), m_fields( 0 ), m_oob( 0 ), m_del( false ),
      m_reg( false )
  {
    if( !tag || tag->name() != "query" || tag->xmlns() != XMLNS_REGISTER )
      return;

    const TagList& l = tag->children();
    TagList::const_iterator it = l.begin();
    for( ; it != l.end(); ++it )
    {
      const std::string& name = (*it)->name();
      if( name == "instructions" )
        m_instructions = (*it)->cdata();
      else if( name == "remove" )
        m_del = true;
      else if( name == "registered" )
        m_reg = true;
      else if( name == "username" )
      {
        m_fields |= FieldUsername;
        m_values.username = (*it)->cdata();
      }
      else if( name == "nick" )
      {
        m_fields |= FieldNick;
        m_values.nick = (*it)->cdata();
      }
      else if( name == "password" )
      {
        m_fields |= FieldPassword;
        m_values.password = (*it)->cdata();
      }
      else if( name == "name" )
      {
        m_fields |= FieldName;
        m_values.name = (*it)->cdata();
      }
      else if( name == "first" )
      {
        m_fields |= FieldFirst;
        m_values.first = (*it)->cdata();
      }
      else if( name == "last" )
      {
        m_fields |= FieldLast;
        m_values.last = (*it)->cdata();
      }
      else if( name == "email" )
      {
        m_fields |= FieldEmail;
        m_values.email = (*it)->cdata();
      }
      else if( name == "address" )
      {
        m_fields |= FieldAddress;
        m_values.address = (*it)->cdata();
      }
      else if( name == "city" )
      {
        m_fields |= FieldCity;
        m_values.city = (*it)->cdata();
      }
      else if( name == "state" )
      {
        m_fields |= FieldState;
        m_values.state = (*it)->cdata();
      }
      else if( name == "zip" )
      {
        m_fields |= FieldZip;
        m_values.zip = (*it)->cdata();
      }
      else if( name == "phone" )
      {
        m_fields |= FieldPhone;
        m_values.phone = (*it)->cdata();
      }
      else if( name == "url" )
      {
        m_fields |= FieldUrl;
        m_values.url = (*it)->cdata();
      }
      else if( name == "date" )
      {
        m_fields |= FieldDate;
        m_values.date = (*it)->cdata();
      }
      else if( name == "misc" )
      {
        m_fields |= FieldMisc;
        m_values.misc = (*it)->cdata();
      }
      else if( name == "text" )
      {
        m_fields |= FieldText;
        m_values.text = (*it)->cdata();
      }
      else if( !m_form && name == "x" && (*it)->xmlns() == XMLNS_X_DATA )
        m_form = new DataForm( (*it) );
      else if( !m_oob && name == "x" && (*it)->xmlns() == XMLNS_X_OOB )
        m_oob = new OOB( (*it) );
    }
  }

  Registration::Query::~Query()
  {
    delete m_form;
    delete m_oob;
  }

  const std::string& Registration::Query::filterString() const
  {
    static const std::string filter = "/iq/query[@xmlns='" + XMLNS_REGISTER + "']";
    return filter;
  }

  Tag* Registration::Query::tag() const
  {
    Tag* t = new Tag( "query" );
    t->setXmlns( XMLNS_REGISTER );

    if( !m_instructions.empty() )
      new Tag( t, "instructions", m_instructions );

    if ( m_reg )
      new Tag( t, "registered" );

    if( m_form )
      t->addChild( m_form->tag() );
    else if( m_oob )
      t->addChild( m_oob->tag() );
    else if( m_del )
      new Tag( t, "remove" );
    else if( m_fields )
    {
      if( m_fields & FieldUsername )
        new Tag( t, "username", m_values.username );
      if( m_fields & FieldNick )
        new Tag( t, "nick", m_values.nick );
      if( m_fields & FieldPassword )
        new Tag( t, "password", m_values.password );
      if( m_fields & FieldName )
        new Tag( t, "name", m_values.name );
      if( m_fields & FieldFirst )
        new Tag( t, "first", m_values.first );
      if( m_fields & FieldLast )
        new Tag( t, "last", m_values.last );
      if( m_fields & FieldEmail )
        new Tag( t, "email", m_values.email );
      if( m_fields & FieldAddress )
        new Tag( t, "address", m_values.address );
      if( m_fields & FieldCity )
        new Tag( t, "city", m_values.city );
      if( m_fields & FieldState )
        new Tag( t, "state", m_values.state );
      if( m_fields & FieldZip )
        new Tag( t, "zip", m_values.zip );
      if( m_fields & FieldPhone )
        new Tag( t, "phone", m_values.phone );
      if( m_fields & FieldUrl )
        new Tag( t, "url", m_values.url );
      if( m_fields & FieldDate )
        new Tag( t, "date", m_values.date );
      if( m_fields & FieldMisc )
        new Tag( t, "misc", m_values.misc );
      if( m_fields & FieldText )
        new Tag( t, "text", m_values.text );
    }

    return t;
  }
  // ---- ~Registration::Query ----

  // ---- Registration ----
  Registration::Registration( ClientBase* parent, const JID& to )
    : m_parent( parent ), m_to( to ), m_registrationHandler( 0 )
  {
    init();
  }

  Registration::Registration( ClientBase* parent )
  : m_parent( parent ), m_registrationHandler( 0 )
  {
    init();
  }

  void Registration::init()
  {
    if( m_parent )
    {
      m_parent->registerIqHandler( this, ExtRegistration );
      m_parent->registerStanzaExtension( new Query() );
    }
  }

  Registration::~Registration()
  {
    if( m_parent )
    {
      m_parent->removeIqHandler( this, ExtRegistration );
      m_parent->removeIDHandler( this );
      m_parent->removeStanzaExtension( ExtRegistration );
    }
  }

  void Registration::fetchRegistrationFields()
  {
    if( !m_parent || m_parent->state() != StateConnected )
      return;

    IQ iq( IQ::Get, m_to );
    iq.addExtension( new Query() );
    m_parent->send( iq, this, FetchRegistrationFields );
  }

  bool Registration::createAccount( int fields, const RegistrationFields& values )
  {
    std::string username;
    if( !m_parent || !prep::nodeprep( values.username, username ) )
      return false;

    IQ iq( IQ::Set, m_to );
    iq.addExtension( new Query( fields, values ) );
    m_parent->send( iq, this, CreateAccount );

    return true;
  }

  void Registration::createAccount( DataForm* form )
  {
    if( !m_parent || !form )
      return;

    IQ iq( IQ::Set, m_to );
    iq.addExtension( new Query( form ) );
    m_parent->send( iq, this, CreateAccount );
  }

  void Registration::removeAccount()
  {
    if( !m_parent || !m_parent->authed() )
      return;

    IQ iq( IQ::Set, m_to );
    iq.addExtension( new Query( true ) );
    m_parent->send( iq, this, RemoveAccount );
  }

  void Registration::changePassword( const std::string& username, const std::string& password )
  {
    if( !m_parent || !m_parent->authed() || username.empty() )
      return;

    int fields = FieldUsername | FieldPassword;
    RegistrationFields rf;
    rf.username = username;
    rf.password = password;
    createAccount( fields, rf );
  }

  void Registration::registerRegistrationHandler( RegistrationHandler* rh )
  {
    m_registrationHandler = rh;
  }

  void Registration::removeRegistrationHandler()
  {
    m_registrationHandler = 0;
  }

  void Registration::handleIqID( const IQ& iq, int context )
  {
    if( !m_registrationHandler )
      return;

    if( iq.subtype() == IQ::Result )
    {
      switch( context )
      {
        case FetchRegistrationFields:
        {
          const Query* q = iq.findExtension<Query>( ExtRegistration );
          if( !q )
            return;

          if( q->registered() )
            m_registrationHandler->handleAlreadyRegistered( iq.from() );

          if( q->form() )
            m_registrationHandler->handleDataForm( iq.from(), *(q->form()) );

          if( q->oob() )
            m_registrationHandler->handleOOB( iq.from(), *(q->oob()) );

          m_registrationHandler->handleRegistrationFields( iq.from(), q->fields(), q->instructions() );
          break;
        }

        case CreateAccount:
        case ChangePassword:
        case RemoveAccount:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationSuccess );
          break;
      }
    }
    else if( iq.subtype() == IQ::Error )
    {
      const Error* e = iq.error();
      if( !e )
        return;

      switch( e->error() )
      {
        case StanzaErrorConflict:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationConflict );
          break;
        case StanzaErrorNotAcceptable:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationNotAcceptable );
          break;
        case StanzaErrorBadRequest:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationBadRequest );
          break;
        case StanzaErrorForbidden:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationForbidden );
          break;
        case StanzaErrorRegistrationRequired:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationRequired );
          break;
        case StanzaErrorUnexpectedRequest:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationUnexpectedRequest );
          break;
        case StanzaErrorNotAuthorized:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationNotAuthorized );
          break;
        case StanzaErrorNotAllowed:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationNotAllowed );
          break;
        default:
          m_registrationHandler->handleRegistrationResult( iq.from(), RegistrationUnknownError );
          break;

      }
    }

  }

}
