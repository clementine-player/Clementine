/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "siprofileft.h"

#include "clientbase.h"
#include "siprofilefthandler.h"
#include "simanager.h"
#include "dataform.h"
#include "inbandbytestream.h"
#include "oob.h"
#include "socks5bytestream.h"
#include "socks5bytestreammanager.h"

#include <cstdlib>
#include <map>

namespace gloox
{

  SIProfileFT::SIProfileFT( ClientBase* parent, SIProfileFTHandler* sipfth, SIManager* manager,
                            SOCKS5BytestreamManager* s5Manager )
    : m_parent( parent ), m_manager( manager ), m_handler( sipfth ),
      m_socks5Manager( s5Manager ), m_delManager( false ),
      m_delS5Manager( false )
  {
    if( !m_manager )
    {
      m_delManager = true;
      m_manager = new SIManager( m_parent );
    }

    m_manager->registerProfile( XMLNS_SI_FT, this );

    if( !m_socks5Manager )
    {
      m_socks5Manager = new SOCKS5BytestreamManager( m_parent, this );
      m_delS5Manager = true;
    }
  }

  SIProfileFT::~SIProfileFT()
  {
    m_manager->removeProfile( XMLNS_SI_FT );

    if( m_delManager )
      delete m_manager;

    if( m_socks5Manager && m_delS5Manager )
      delete m_socks5Manager;
  }

  const std::string SIProfileFT::requestFT( const JID& to, const std::string& name, long size,
                                            const std::string& hash, const std::string& desc,
                                            const std::string& date, const std::string& mimetype,
                                            int streamTypes, const JID& from,
                                            const std::string& sid )
  {
    if( name.empty() || size <= 0 || !m_manager )
      return EmptyString;

    Tag* file = new Tag( "file", XMLNS, XMLNS_SI_FT );
    file->addAttribute( "name", name );
    file->addAttribute( "size", size );
    if( !hash.empty() )
      file->addAttribute( "hash", hash );
    if( !date.empty() )
      file->addAttribute( "date", date );
    if( !desc.empty() )
      new Tag( file, "desc", desc );

    Tag* feature = new Tag( "feature", XMLNS, XMLNS_FEATURE_NEG );
    DataForm df( TypeForm );
    DataFormField* dff = df.addField( DataFormField::TypeListSingle, "stream-method" );
    StringMultiMap sm;
    if( streamTypes & FTTypeS5B )
      sm.insert( std::make_pair( "s5b", XMLNS_BYTESTREAMS ) );
    if( streamTypes & FTTypeIBB )
      sm.insert( std::make_pair( "ibb", XMLNS_IBB ) );
    if( streamTypes & FTTypeOOB )
      sm.insert( std::make_pair( "oob", XMLNS_IQ_OOB ) );
    dff->setOptions( sm );
    feature->addChild( df.tag() );

    return m_manager->requestSI( this, to, XMLNS_SI_FT, file, feature, mimetype, from, sid );
  }

  void SIProfileFT::acceptFT( const JID& to, const std::string& sid, StreamType type, const JID& from )
  {
    if( !m_manager )
      return;

    if( m_id2sid.find( sid ) == m_id2sid.end() )
      return;

    const std::string& id = m_id2sid[sid];

    Tag* feature = new Tag( "feature", XMLNS, XMLNS_FEATURE_NEG );
    DataFormField* dff = new DataFormField( "stream-method" );
    switch( type )
    {
      case FTTypeAll:
      case FTTypeS5B:
        dff->setValue( XMLNS_BYTESTREAMS );
        break;
      case FTTypeIBB:
        dff->setValue( XMLNS_IBB );
        if( m_handler )
        {
          InBandBytestream* ibb = new InBandBytestream( m_parent, m_parent->logInstance(), to,
                                                        from ? from : m_parent->jid(), sid );
          m_handler->handleFTBytestream( ibb );
        }
        break;
      case FTTypeOOB:
        dff->setValue( XMLNS_IQ_OOB );
        break;
    }
    DataForm df( TypeSubmit );
    df.addField( dff );
    feature->addChild( df.tag() );

    m_manager->acceptSI( to, id, 0, feature, from );
  }

  void SIProfileFT::declineFT( const JID& to, const std::string& sid, SIManager::SIError reason,
                               const std::string& text )
  {
    if( m_id2sid.find( sid ) == m_id2sid.end() || !m_manager )
      return;

    m_manager->declineSI( to, m_id2sid[sid], reason, text );
  }

  void SIProfileFT::dispose( Bytestream* bs )
  {
    if( bs )
    {
      if( bs->type() == Bytestream::S5B && m_socks5Manager )
        m_socks5Manager->dispose( static_cast<SOCKS5Bytestream*>( bs ) );
      else
        delete bs;
    }
  }

  void SIProfileFT::cancel( Bytestream* bs )
  {
    if( !bs )
      return;

    if( m_id2sid.find( bs->sid() ) == m_id2sid.end() || !m_manager )
      return;

    if( bs->type() == Bytestream::S5B && m_socks5Manager )
      m_socks5Manager->rejectSOCKS5Bytestream( bs->sid(), StanzaErrorServiceUnavailable );

    dispose( bs );
  }

  void SIProfileFT::setStreamHosts( StreamHostList hosts )
  {
    if( m_socks5Manager )
      m_socks5Manager->setStreamHosts( hosts );
  }

  void SIProfileFT::addStreamHost( const JID& jid, const std::string& host, int port )
  {
    if( m_socks5Manager )
      m_socks5Manager->addStreamHost( jid, host, port );
  }

  void SIProfileFT::handleSIRequest( const JID& from, const JID& to, const std::string& id,
                                     const SIManager::SI& si )
  {
    if( si.profile() != XMLNS_SI_FT || !si.tag1() )
      return;

    if( m_handler )
    {
      const Tag* t = si.tag1()->findChild( "desc" );
      const std::string& desc = t ? t->cdata() : EmptyString;

      const std::string& mt = si.mimetype();
      int types = 0;

      if( si.tag2() )
      {
        const DataForm df( si.tag2()->findChild( "x", XMLNS, XMLNS_X_DATA ) );
        const DataFormField* dff = df.field( "stream-method" );

        if( dff )
        {
          const StringMultiMap& options = dff->options();
          StringMultiMap::const_iterator it = options.begin();
          for( ; it != options.end(); ++it )
          {
            if( (*it).second == XMLNS_BYTESTREAMS )
              types |= FTTypeS5B;
            else if( (*it).second == XMLNS_IBB )
              types |= FTTypeIBB;
            else if( (*it).second == XMLNS_IQ_OOB )
              types |= FTTypeOOB;
          }
        }
      }

      const std::string& sid = si.id();
      m_id2sid[sid] = id;
      m_handler->handleFTRequest( from, to, sid, si.tag1()->findAttribute( "name" ),
                                  atol( si.tag1()->findAttribute( "size" ).c_str() ),
                                        si.tag1()->findAttribute( "hash" ),
                                            si.tag1()->findAttribute( "date" ),
                                  mt.empty() ? "binary/octet-stream" : mt,
                                  desc, types );
    }
  }

  void SIProfileFT::handleSIRequestResult( const JID& from, const JID& to, const std::string& sid,
                                           const SIManager::SI& si )
  {
    if( si.tag2() )
    {
      const DataForm df( si.tag2()->findChild( "x", XMLNS, XMLNS_X_DATA ) );
      const DataFormField* dff = df.field( "stream-method" );

      if( dff )
      {
        if( m_socks5Manager && dff->value() == XMLNS_BYTESTREAMS )
        {
          // check return value:
          m_socks5Manager->requestSOCKS5Bytestream( from, SOCKS5BytestreamManager::S5BTCP, sid, to );
        }
        else if( m_handler )
        {
          if( dff->value() == XMLNS_IBB )
          {
            InBandBytestream* ibb = new InBandBytestream( m_parent, m_parent->logInstance(),
                to ? to : m_parent->jid(), from, sid );

            m_handler->handleFTBytestream( ibb );
          }
          else if( dff->value() == XMLNS_IQ_OOB )
          {
            const std::string& url = m_handler->handleOOBRequestResult( from, to, sid );
            if( !url.empty() )
            {
              const std::string& id = m_parent->getID();
              IQ iq( IQ::Set, from, id );
              if( to )
                iq.setFrom( to );

              iq.addExtension( new OOB( url, EmptyString, true ) );
              m_parent->send( iq, this, OOBSent );
            }
          }
        }
      }
    }
  }

  void SIProfileFT::handleIqID( const IQ& /*iq*/, int context )
  {
    switch( context )
    {
      case OOBSent:
//         if( iq->subtype() == IQ::Error )
//           m_handler->handleOOBError
        break;
    }
  }

  void SIProfileFT::handleSIRequestError( const IQ& iq, const std::string& sid )
  {
    if( m_handler )
      m_handler->handleFTRequestError( iq, sid );
  }

  void SIProfileFT::handleIncomingBytestreamRequest( const std::string& sid, const JID& /*from*/ )
  {
// TODO: check for valid sid/from tuple
    m_socks5Manager->acceptSOCKS5Bytestream( sid );
  }

  void SIProfileFT::handleIncomingBytestream( Bytestream* bs )
  {
    if( m_handler )
      m_handler->handleFTBytestream( bs );
  }

  void SIProfileFT::handleOutgoingBytestream( Bytestream* bs )
  {
    if( m_handler )
      m_handler->handleFTBytestream( bs );
  }

  void SIProfileFT::handleBytestreamError( const IQ& iq, const std::string& sid )
  {
    if( m_handler )
      m_handler->handleFTRequestError( iq, sid );
  }

}
