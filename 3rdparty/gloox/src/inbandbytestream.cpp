/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "inbandbytestream.h"
#include "base64.h"
#include "bytestreamdatahandler.h"
#include "disco.h"
#include "clientbase.h"
#include "error.h"
#include "message.h"
#include "util.h"

#include <cstdlib>

namespace gloox
{

  // ---- InBandBytestream::IBB ----
  static const char* typeValues[] =
  {
    "open", "data", "close"
  };

  InBandBytestream::IBB::IBB( const std::string& sid, int blocksize )
    : StanzaExtension( ExtIBB ), m_sid ( sid ), m_seq( 0 ), m_blockSize( blocksize ),
      m_type( IBBOpen )
  {
  }

  InBandBytestream::IBB::IBB( const std::string& sid, int seq, const std::string& data )
    : StanzaExtension( ExtIBB ), m_sid ( sid ), m_seq( seq ), m_blockSize( 0 ),
      m_data( data ), m_type( IBBData )
  {
  }

  InBandBytestream::IBB::IBB( const std::string& sid )
    : StanzaExtension( ExtIBB ), m_sid ( sid ), m_seq( 0 ), m_blockSize( 0 ),
      m_type( IBBClose )
  {
  }

  InBandBytestream::IBB::IBB( const Tag* tag )
    : StanzaExtension( ExtIBB ), m_type( IBBInvalid )
  {
    if( !tag || tag->xmlns() != XMLNS_IBB )
      return;

    m_type = (IBBType)util::lookup( tag->name(), typeValues );
    m_blockSize = atoi( tag->findAttribute( "block-size" ).c_str() );
    m_seq = atoi( tag->findAttribute( "seq" ).c_str() );
    m_sid = tag->findAttribute( "sid" );
    m_data = Base64::decode64( tag->cdata() );
  }

  InBandBytestream::IBB::~IBB()
  {
  }

  const std::string& InBandBytestream::IBB::filterString() const
  {
    static const std::string filter = "/iq/open[@xmlns='" + XMLNS_IBB + "']"
                                      "|/iq/data[@xmlns='" + XMLNS_IBB + "']"
                                      "|/message/data[@xmlns='" + XMLNS_IBB + "']"
                                      "|/iq/close[@xmlns='" + XMLNS_IBB + "']";
    return filter;
  }

  Tag* InBandBytestream::IBB::tag() const
  {
    if( m_type == IBBInvalid )
      return 0;

    Tag* t = new Tag( util::lookup( m_type, typeValues ) );
    t->setXmlns( XMLNS_IBB );
    t->addAttribute( "sid", m_sid );
    if( m_type == IBBData )
    {
      t->setCData( Base64::encode64( m_data ) );
      t->addAttribute( "seq", m_seq );
    }
    else if( m_type == IBBOpen )
      t->addAttribute( "block-size", m_blockSize );

    return t;
  }
  // ---- ~InBandBytestream::IBB ----

  // ---- InBandBytestream ----
  InBandBytestream::InBandBytestream( ClientBase* clientbase, LogSink& logInstance, const JID& initiator,
                                      const JID& target, const std::string& sid )
    : Bytestream( Bytestream::IBB, logInstance, initiator, target, sid ),
      m_clientbase( clientbase ), m_blockSize( 4096 ), m_sequence( -1 ), m_lastChunkReceived( -1 )
  {
    if( m_clientbase )
    {
      m_clientbase->registerStanzaExtension( new IBB() );
      m_clientbase->registerIqHandler( this, ExtIBB );
      m_clientbase->registerMessageHandler( this );
    }

    m_open = false;
  }

  InBandBytestream::~InBandBytestream()
  {
    if( m_open )
      close();

    if( m_clientbase )
    {
      m_clientbase->removeMessageHandler( this );
      m_clientbase->removeIqHandler( this, ExtIBB );
      m_clientbase->removeIDHandler( this );
    }
  }

  bool InBandBytestream::connect()
  {
    if( !m_clientbase )
      return false;

    if( m_target == m_clientbase->jid() )
      return true;

    const std::string& id = m_clientbase->getID();
    IQ iq( IQ::Set, m_target, id );
    iq.addExtension( new IBB( m_sid, m_blockSize ) );
    m_clientbase->send( iq, this, IBBOpen );
    return true;
  }

  void InBandBytestream::handleIqID( const IQ& iq, int context )
  {
    switch( iq.subtype() )
    {
      case IQ::Result:
        if( context == IBBOpen && m_handler )
        {
          m_handler->handleBytestreamOpen( this );
          m_open = true;
        }
        break;
      case IQ::Error:
        closed();
        break;
      default:
        break;
    }
  }

  bool InBandBytestream::handleIq( const IQ& iq ) // data or open request, always 'set'
  {
    const IBB* i = iq.findExtension<IBB>( ExtIBB );
    if( !i || !m_handler || iq.subtype() != IQ::Set )
      return false;

    if( !m_open )
    {
      if( i->type() == IBBOpen )
      {
        returnResult( iq.from(), iq.id() );
        m_open = true;
        m_handler->handleBytestreamOpen( this );
        return true;
      }
      return false;
    }

    if( i->type() == IBBClose )
    {
      returnResult( iq.from(), iq.id() );
      closed();
      return true;
    }

    if( ( m_lastChunkReceived + 1 ) != i->seq() )
    {
      m_open = false;
      returnError( iq.from(), iq.id(), StanzaErrorTypeModify, StanzaErrorItemNotFound );
      return false;
    }

    if( i->data().empty() )
    {
      m_open = false;
      returnError( iq.from(), iq.id(), StanzaErrorTypeModify, StanzaErrorBadRequest );
      return false;
    }

    returnResult( iq.from(), iq.id() );
    m_handler->handleBytestreamData( this, i->data() );
    m_lastChunkReceived++;
    return true;
  }

  void InBandBytestream::handleMessage( const Message& msg, MessageSession* /*session*/ )
  {
    if( msg.from() != m_target || !m_handler )
      return;

    const IBB* i = msg.findExtension<IBB>( ExtIBB );
    if( !i )
      return;

    if( !m_open )
      return;

    if( m_lastChunkReceived != i->seq() )
    {
      m_open = false;
      return;
    }

    if( i->data().empty() )
    {
      m_open = false;
      return;
    }

    m_handler->handleBytestreamData( this, i->data() );
    m_lastChunkReceived++;
  }

  void InBandBytestream::returnResult( const JID& to, const std::string& id )
  {
    IQ iq( IQ::Result, to, id );
    m_clientbase->send( iq );
  }

  void InBandBytestream::returnError( const JID& to, const std::string& id, StanzaErrorType type, StanzaError error )
  {
    IQ iq( IQ::Error, to, id );
    iq.addExtension( new Error( type, error ) );
    m_clientbase->send( iq );
  }

  bool InBandBytestream::send( const std::string& data )
  {
    if( !m_open || !m_clientbase )
      return false;

    size_t pos = 0;
    size_t len = data.length();
    do
    {
      const std::string& id = m_clientbase->getID();
      IQ iq( IQ::Set, m_target, id );
      iq.addExtension( new IBB( m_sid, ++m_sequence, data.substr( pos, m_blockSize ) ) );
      m_clientbase->send( iq, this, IBBData );

      pos += m_blockSize;
      if( m_sequence == 65535 )
        m_sequence = -1;
    }
    while( pos < len );

    return true;
  }

  void InBandBytestream::closed()
  {
    if( !m_open )
      return;

    m_open = false;

    if( m_handler )
      m_handler->handleBytestreamClose( this );
  }

  void InBandBytestream::close()
  {
    m_open = false;

    if( !m_clientbase )
      return;

    const std::string& id = m_clientbase->getID();
    IQ iq( IQ::Set, m_target, id );
    iq.addExtension( new IBB( m_sid ) );
    m_clientbase->send( iq, this, IBBClose );

    if( m_handler )
      m_handler->handleBytestreamClose( this );
  }

}
