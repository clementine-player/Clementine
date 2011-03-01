/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "amp.h"
#include "tag.h"
#include "util.h"

namespace gloox
{

  static const char* conditionValues[] =
  {
    "deliver", "expire-at", "match-resource"
  };

  static const char* actionValues[] =
  {
    "alert", "error", "drop", "notify"
  };

  static const char* deliverValues[] =
  {
    "direct", "forward", "gateway", "none", "stored"
  };

  static const char* matchResourceValues[] =
  {
    "any", "exact", "other"
  };

  static const char* statusValues[] =
  {
    "alert", "notify"
  };

  // ---- AMP::Rule ----
  AMP::Rule::Rule( DeliverType deliver, ActionType action )
    : m_condition( ConditionDeliver ), m_deliver( deliver ), m_action( action )
  {
  }

  AMP::Rule::Rule( const std::string& date, ActionType action )
    : m_condition( ConditionExpireAt ), m_expireat( new std::string( date ) ), m_action( action )
  {
  }

  AMP::Rule::Rule( MatchResourceType match, ActionType action )
    : m_condition( ConditionMatchResource ), m_matchresource( match ), m_action( action )
  {
  }

  AMP::Rule::Rule( const std::string& condition, const std::string& action,
                   const std::string& value )
  {
    m_condition = (ConditionType)util::lookup( condition, conditionValues );
    m_action = (ActionType)util::lookup( action, actionValues );
    switch( m_condition )
    {
      case ConditionDeliver:
        m_deliver = (DeliverType)util::lookup( value, deliverValues );
        break;
      case ConditionExpireAt:
        m_expireat = new std::string( value );
        break;
      case ConditionMatchResource:
        m_matchresource = (MatchResourceType)util::lookup( value, matchResourceValues );
        break;
      default:
      case ConditionInvalid: // shouldn't happen
        break;
    }
  }

  AMP::Rule::~Rule()
  {
    if( m_condition == ConditionExpireAt && m_expireat )
      delete m_expireat;
  }

  Tag* AMP::Rule::tag() const
  {
    if( m_condition == ConditionInvalid || m_action == ActionInvalid
        || ( m_condition == ConditionDeliver && m_deliver == DeliverInvalid )
        || ( m_condition == ConditionMatchResource && m_matchresource == MatchResourceInvalid )
        || ( m_condition == ConditionExpireAt && !m_expireat ) )
     return 0;

    Tag* rule = new Tag( "rule" );
    rule->addAttribute( "condition", util::lookup( m_condition, conditionValues ) );
    rule->addAttribute( "action", util::lookup( m_action, actionValues ) );

    switch( m_condition )
    {
      case ConditionDeliver:
        rule->addAttribute( "value", util::lookup( m_deliver, deliverValues ) );
        break;
      case ConditionExpireAt:
        rule->addAttribute( "value", *m_expireat );
        break;
      case ConditionMatchResource:
        rule->addAttribute( "value", util::lookup( m_matchresource, matchResourceValues ) );
        break;
      default:
        break;
    }
    return rule;
  }
  // ---- AMP::Rule ----

  // ---- AMP ----
  AMP::AMP( bool perhop )
    : StanzaExtension( ExtAMP ), m_perhop( perhop ), m_status( StatusInvalid )
  {
    m_valid = true;
  }

  AMP::AMP( const Tag* tag )
    : StanzaExtension( ExtAMP ), m_perhop( false )
  {
    if( !tag || tag->name() != "amp" || tag->xmlns() != XMLNS_AMP )
      return;

    const ConstTagList& rules = tag->findTagList( "/amp/rule" );
    ConstTagList::const_iterator it = rules.begin();
    for( ; it != rules.end(); ++it )
    {
      m_rules.push_back( new Rule( (*it)->findAttribute( "condition" ),
                                   (*it)->findAttribute( "action" ),
                                   (*it)->findAttribute( "value" ) ) );
    }

    m_from = tag->findAttribute( "from" );
    m_to = tag->findAttribute( "to" );
    m_status = (Status)util::lookup( tag->findAttribute( "status" ), statusValues );
    if( tag->hasAttribute( "per-hop", "true" ) || tag->hasAttribute( "per-hop", "1" ) )
      m_perhop = true;
    m_valid = true;
  }

  AMP::~AMP()
  {
    util::clearList( m_rules );
  }

  void AMP::addRule( const Rule* rule )
  {
    if( rule )
      m_rules.push_back( rule );
  }

  const std::string& AMP::filterString() const
  {
    static const std::string filter = "/message/amp[@xmlns='" + XMLNS_AMP + "']";
    return filter;
  }

  Tag* AMP::tag() const
  {
    if( !m_valid || !m_rules.size() )
      return 0;

    Tag* amp = new Tag( "amp" );
    amp->setXmlns( XMLNS_AMP );
    if( m_from )
      amp->addAttribute( "from", m_from.full() );
    if( m_to )
      amp->addAttribute( "to", m_to.full() );
    if( m_status != StatusInvalid )
      amp->addAttribute( "status", util::lookup( m_status, statusValues ) );
    if( m_perhop )
      amp->addAttribute( "per-hop", "true" );
    RuleList::const_iterator it = m_rules.begin();
    for( ; it != m_rules.end(); ++it )
      amp->addChild( (*it)->tag() );

    return amp;
  }

}
