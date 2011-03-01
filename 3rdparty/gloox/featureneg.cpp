/*
  Copyright (c) 2007-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "featureneg.h"
#include "dataform.h"
#include "tag.h"

namespace gloox
{

  FeatureNeg::FeatureNeg( DataForm* form )
    : StanzaExtension( ExtFeatureNeg ), m_form( form )
  {
  }

  FeatureNeg::FeatureNeg( const Tag* tag )
    : StanzaExtension( ExtFeatureNeg ), m_form( 0 )
  {
    if( !tag || tag->name() != "feature" || tag->xmlns() != XMLNS_FEATURE_NEG )
      return;

    const Tag* f = tag->findTag( "feature/x[@xmlns='" + XMLNS_X_DATA + "']" );
    if( f )
      m_form = new DataForm( f );
  }

  FeatureNeg::~FeatureNeg()
  {
    delete m_form;
  }

  const std::string& FeatureNeg::filterString() const
  {
    static const std::string filter = "/message/feature[@xmlns='" + XMLNS_FEATURE_NEG + "']"
                                      "|/iq/feature[@xmlns='" + XMLNS_FEATURE_NEG + "']" ;
    return filter;
  }

  Tag* FeatureNeg::tag() const
  {
    if( !m_form )
      return 0;

    Tag* t = new Tag( "feature" );
    t->setXmlns( XMLNS_FEATURE_NEG );
    t->addChild( m_form->tag() );
    return t;
  }

}
