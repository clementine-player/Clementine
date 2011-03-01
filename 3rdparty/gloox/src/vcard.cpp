/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "vcard.h"
#include "tag.h"
#include "base64.h"

namespace gloox
{

  void VCard::insertField( Tag* vcard, const char* field, const std::string& var )
  {
    if( field && !var.empty() )
      new Tag( vcard, field, var );
  }

  void VCard::insertField( Tag* vcard, const char* field, bool var )
  {
    if( field && var )
      new Tag( vcard, field );
  }

  void VCard::checkField( const Tag* vcard, const char* field, std::string& var )
  {
    if( field )
    {
      Tag* child = vcard->findChild( field );
      if( child )
        var = child->cdata();
    }
  }

  VCard::VCard()
    : StanzaExtension( ExtVCard ), m_class( ClassNone ), m_prodid( "gloox" + GLOOX_VERSION ),
      m_N( false ), m_PHOTO( false ), m_LOGO( false )
  {
    m_valid = true;
  }

  VCard::VCard( const Tag* vcard )
    : StanzaExtension( ExtVCard ), m_class( ClassNone ), m_prodid( "gloox" + GLOOX_VERSION ),
      m_N( false ), m_PHOTO( false ), m_LOGO( false )
  {
    if( !vcard || vcard->name() != "vCard" || vcard->xmlns() != XMLNS_VCARD_TEMP )
      return;

    m_valid = true;

    checkField( vcard, "FN", m_formattedname );
    checkField( vcard, "NICKNAME", m_nickname );
    checkField( vcard, "URL", m_url );
    checkField( vcard, "BDAY", m_bday );
    checkField( vcard, "JABBERID", m_jabberid );
    checkField( vcard, "TITLE", m_title );
    checkField( vcard, "ROLE", m_role );
    checkField( vcard, "NOTE", m_note );
    checkField( vcard, "DESC", m_desc );
    checkField( vcard, "MAILER", m_mailer );
    checkField( vcard, "TZ", m_tz );
    checkField( vcard, "PRODID", m_prodid );
    checkField( vcard, "REV", m_rev );
    checkField( vcard, "SORT-STRING", m_sortstring );
    checkField( vcard, "UID", m_uid );

    TagList::const_iterator it = vcard->children().begin();
    for( ; it != vcard->children().end(); ++it )
    {
      const Tag& tag = *(*it);
      if( tag.name() == "N" )
      {
        m_N = true;
        const Tag * child = tag.findChild( "FAMILY" );
        if( child )
          m_name.family = child->cdata();
        if( ( child = tag.findChild( "GIVEN" ) ) )
          m_name.given = child->cdata();
        if( ( child = tag.findChild( "MIDDLE" ) ) )
          m_name.middle = child->cdata();
        if( ( child = tag.findChild( "PREFIX" ) ) )
          m_name.prefix = child->cdata();
        if( ( child = tag.findChild( "SUFFIX" ) ) )
          m_name.suffix = child->cdata();
      }
      else if( tag.name() == "PHOTO" )
      {
        if( tag.hasChild( "EXTVAL" ) )
        {
          m_photo.extval = tag.findChild( "EXTVAL" )->cdata();
          m_PHOTO = true;
        }
        else if( tag.hasChild( "TYPE" ) && tag.hasChild( "BINVAL" ) )
        {
          std::string binval = tag.findChild( "BINVAL" )->cdata();
          std::string::size_type pos = 0;
          while( ( pos = binval.find( '\n' ) ) != std::string::npos )
            binval.erase( pos, 1 );
          m_photo.type = tag.findChild( "TYPE" )->cdata();
          m_photo.binval = Base64::decode64( binval );
          m_PHOTO = true;
        }
      }
      else if( tag.name() == "LOGO" )
      {
        if( tag.hasChild( "EXTVAL" ) )
        {
          m_logo.extval = tag.findChild( "EXTVAL" )->cdata();
          m_LOGO = true;
        }
        else if( tag.hasChild( "TYPE" ) && tag.hasChild( "BINVAL" ) )
        {
          std::string binval = tag.findChild( "BINVAL" )->cdata();
          std::string::size_type pos = 0;
          while( ( pos = binval.find( '\n' ) ) != std::string::npos )
            binval.erase( pos, 1 );
          m_logo.type = tag.findChild( "TYPE" )->cdata();
          m_logo.binval = Base64::decode64( binval );
          m_LOGO = true;
        }
      }
      else if( tag.name() == "EMAIL" && tag.hasChild( "USERID" ) )
      {
        Email item;
        item.userid = tag.findChild( "USERID" )->cdata();
        item.internet = tag.hasChild( "INTERNET" );
        item.x400 = tag.hasChild( "X400" );
        item.work = tag.hasChild( "WORK" );
        item.home = tag.hasChild( "HOME" );
        item.pref = tag.hasChild( "PREF" );
        m_emailList.push_back( item );
      }
      else if( tag.name() == "ADR" )
      {
        Address item;
        checkField( &tag, "POBOX", item.pobox );
        checkField( &tag, "EXTADD", item.extadd );
        checkField( &tag, "STREET", item.street );
        checkField( &tag, "LOCALITY", item.locality );
        checkField( &tag, "REGION", item.region );
        checkField( &tag, "PCODE", item.pcode );
        checkField( &tag, "CTRY", item.ctry );
        item.postal = tag.hasChild( "POSTAL" );
        item.parcel = tag.hasChild( "PARCEL" );
        item.work = tag.hasChild( "WORK" );
        item.home = tag.hasChild( "HOME" );
        item.pref = tag.hasChild( "PREF" );
        item.dom = tag.hasChild( "DOM" );
        item.intl = !item.dom && tag.hasChild( "INTL" );
        m_addressList.push_back( item );
      }
      else if( tag.name() == "LABEL" )
      {
        Label item;
        TagList::const_iterator it2 = tag.children().begin();
        for( ; it2 != tag.children().end(); ++it2 )
        {
          if( (*it2)->name() == "LINE" )
            item.lines.push_back( (*it)->cdata() );
          item.postal = (*it2)->name() == "POSTAL";
          item.parcel = (*it2)->name() == "PARCEL";
          item.work = (*it2)->name() == "WORK";
          item.home = (*it2)->name() == "HOME";
          item.pref = (*it2)->name() == "PREF";
          item.dom = (*it2)->name() == "DOM";
          item.intl = !item.dom && (*it2)->name() == "INTL";
        }
        m_labelList.push_back( item );
      }
      else if( tag.name() == "TEL" && tag.hasChild( "NUMBER" ) )
      {
        Telephone item;
        item.number = tag.findChild( "NUMBER" )->cdata();
        item.work = tag.hasChild( "WORK" );
        item.home = tag.hasChild( "HOME" );
        item.voice = tag.hasChild( "VOICE" );
        item.fax = tag.hasChild( "FAX" );
        item.pager = tag.hasChild( "PAGER" );
        item.msg = tag.hasChild( "MSG" );
        item.cell = tag.hasChild( "CELL" );
        item.video = tag.hasChild( "VIDEO" );
        item.bbs = tag.hasChild( "BBS" );
        item.modem = tag.hasChild( "MODEM" );
        item.isdn = tag.hasChild( "ISDN" );
        item.pcs = tag.hasChild( "PCS" );
        item.pref = tag.hasChild( "PREF" );
        m_telephoneList.push_back( item );
      }
      else if( tag.name() == "ORG" )
      {
        TagList::const_iterator ito = tag.children().begin();
        for( ; ito != tag.children().end(); ++ito )
        {
          if( (*ito)->name() == "ORGNAME" )
            m_org.name = (*ito)->cdata();
          else if( (*ito)->name() == "ORGUNIT" )
            m_org.units.push_back( (*ito)->cdata() );
        }
      }
      else if( tag.name() == "GEO" )
      {
        checkField( &tag, "LON", m_geo.longitude );
        checkField( &tag, "LAT", m_geo.latitude );
      }
      else if( tag.name() == "CLASS" )
      {
        if( tag.hasChild( "PRIVATE" ) )
          m_class = ClassPrivate;
        else if( tag.hasChild( "PUBLIC" ) )
          m_class = ClassPublic;
        else if( tag.hasChild( "CONFIDENTIAL" ) )
          m_class = ClassConfidential;
      }

    }

  }

  void VCard::setName( const std::string& family, const std::string& given,
                       const std::string& middle, const std::string& prefix,
                       const std::string& suffix )
  {
    m_name.family = family;
    m_name.given = given;
    m_name.middle = middle;
    m_name.prefix = prefix;
    m_name.suffix = suffix;
    m_N = true;
  }

  void VCard::setPhoto( const std::string& extval )
  {
    if( !extval.empty() )
    {
      m_photo.extval= extval;
      m_PHOTO = true;
    }
  }

  void VCard::setPhoto( const std::string& type, const std::string& binval )
  {
    if( !type.empty() && !binval.empty() )
    {
      m_photo.type = type;
      m_photo.binval = binval;
      m_PHOTO = true;
    }
    else
    {
      m_photo.type = EmptyString;
      m_photo.binval = EmptyString;
      m_photo.extval = EmptyString;
      m_PHOTO = false;
    }
  }

  void VCard::setLogo( const std::string& extval )
  {
    if( !extval.empty() )
    {
      m_logo.extval = extval;
      m_LOGO = true;
    }
  }

  void VCard::setLogo( const std::string& type, const std::string& binval )
  {
    if( !type.empty() && !binval.empty() )
    {
      m_logo.type = type;
      m_logo.binval = binval;
      m_LOGO = true;
    }
    else
    {
      m_logo.type = EmptyString;
      m_logo.binval = EmptyString;
      m_logo.extval = EmptyString;
      m_LOGO = false;
    }
  }

  void VCard::addEmail( const std::string& userid, int type )
  {
    if( userid.empty() )
      return;

    Email item;
    item.userid = userid;
    item.internet = ((type & AddrTypeInet) == AddrTypeInet);
    item.x400 = ((type & AddrTypeX400) == AddrTypeX400);
    item.work = ((type & AddrTypeWork) == AddrTypeWork);
    item.home = ((type & AddrTypeHome) == AddrTypeHome);
    item.pref = ((type & AddrTypePref) == AddrTypePref);

    m_emailList.push_back( item );
  }

  void VCard::addAddress( const std::string& pobox, const std::string& extadd,
                          const std::string& street, const std::string& locality,
                          const std::string& region, const std::string& pcode,
                          const std::string& ctry, int type )
  {
    if( pobox.empty() && extadd.empty() && street.empty() &&
        locality.empty() && region.empty() && pcode.empty() && ctry.empty() )
      return;

    Address item;
    item.pobox = pobox;
    item.extadd = extadd;
    item.street = street;
    item.locality = locality;
    item.region = region;
    item.pcode = pcode;
    item.ctry = ctry;
    item.home = ((type & AddrTypeHome) == AddrTypeHome);
    item.work = ((type & AddrTypeWork) == AddrTypeWork);
    item.parcel = ((type & AddrTypeParcel) == AddrTypeParcel);
    item.postal = ((type & AddrTypePostal) == AddrTypePostal);
    item.dom = ((type & AddrTypeDom) == AddrTypeDom);
    item.intl = !item.dom && ((type & AddrTypeIntl) == AddrTypeIntl);
    item.pref = ((type & AddrTypePref) == AddrTypePref);

    m_addressList.push_back( item );
  }

  void VCard::addLabel( const StringList& lines, int type )
  {
    if( lines.empty() )
      return;

    Label item;
    item.lines = lines;
    item.work = ((type & AddrTypeWork) == AddrTypeWork);
    item.home = ((type & AddrTypeHome) == AddrTypeHome);
    item.postal = ((type & AddrTypePostal) == AddrTypePostal);
    item.parcel = ((type & AddrTypeParcel) == AddrTypeParcel);
    item.pref = ((type & AddrTypePref) == AddrTypePref);
    item.dom = ((type & AddrTypeDom) == AddrTypeDom);
    item.intl = !item.dom && ((type & AddrTypeIntl) == AddrTypeIntl);

    m_labelList.push_back( item );
  }

  void VCard::addTelephone( const std::string& number, int type )
  {
    if( number.empty() )
      return;

    Telephone item;
    item.number = number;
    item.work = ((type & AddrTypeWork) == AddrTypeWork);
    item.home = ((type & AddrTypeHome) == AddrTypeHome);
    item.voice = ((type & AddrTypeVoice) == AddrTypeVoice);
    item.fax = ((type & AddrTypeFax) == AddrTypeFax);
    item.pager = ((type & AddrTypePager) == AddrTypePager);
    item.msg = ((type & AddrTypeMsg) == AddrTypeMsg);
    item.cell = ((type & AddrTypeCell) == AddrTypeCell);
    item.video = ((type & AddrTypeVideo) == AddrTypeVideo);
    item.bbs = ((type & AddrTypeBbs) == AddrTypeBbs);
    item.modem = ((type & AddrTypeModem) == AddrTypeModem);
    item.isdn = ((type & AddrTypeIsdn) == AddrTypeIsdn);
    item.pcs = ((type & AddrTypePcs) == AddrTypePcs);
    item.pref = ((type & AddrTypePref) == AddrTypePref);

    m_telephoneList.push_back( item );
  }

  void VCard::setGeo( const std::string& lat, const std::string& lon )
  {
    if( !lat.empty() && !lon.empty() )
    {
      m_geo.latitude = lat;
      m_geo.longitude = lon;
    }
  }

  void VCard::setOrganization( const std::string& orgname, const StringList& orgunits )
  {
    if( !orgname.empty() )
    {
      m_org.name = orgname;
      m_org.units = orgunits;
    }
  }

  const std::string& VCard::filterString() const
  {
    static const std::string filter = "/iq/vCard[@xmlns='" + XMLNS_VCARD_TEMP + "']";
    return filter;
  }

  Tag* VCard::tag() const
  {
    Tag* v = new Tag( "vCard" );
    v->setXmlns( XMLNS_VCARD_TEMP );

    if( !m_valid )
      return v;

    v->addAttribute( "version", "3.0" );

    insertField( v, "FN", m_formattedname );
    insertField( v, "NICKNAME", m_nickname );
    insertField( v, "URL", m_url );
    insertField( v, "BDAY", m_bday );
    insertField( v, "JABBERID", m_jabberid );
    insertField( v, "TITLE", m_title );
    insertField( v, "ROLE", m_role );
    insertField( v, "NOTE", m_note );
    insertField( v, "DESC", m_desc );
    insertField( v, "MAILER", m_mailer );
    insertField( v, "TZ", m_tz );
    insertField( v, "REV", m_rev );
    insertField( v, "SORT_STRING", m_sortstring );
    insertField( v, "UID", m_uid );

    if( m_N )
    {
      Tag* n = new Tag( v, "N" );
      insertField( n, "FAMILY", m_name.family );
      insertField( n, "GIVEN", m_name.given );
      insertField( n, "MIDDLE", m_name.middle );
      insertField( n, "PREFIX", m_name.prefix );
      insertField( n, "SUFFIX", m_name.suffix );
    }

    if( m_PHOTO )
    {
      Tag* p = new Tag( v, "PHOTO" );
      if( !m_photo.extval.empty() )
      {
        new Tag( p, "EXTVAL", m_photo.extval );
      }
      else if( !m_photo.type.empty() && !m_photo.binval.empty() )
      {
        new Tag( p, "TYPE", m_photo.type );
        new Tag( p, "BINVAL", Base64::encode64( m_photo.binval ) );
      }
    }

    if( m_LOGO )
    {
      Tag* l = new Tag( v, "LOGO" );
      if( !m_logo.extval.empty() )
      {
        new Tag( l, "EXTVAL", m_logo.extval );
      }
      else if( !m_logo.type.empty() && !m_logo.binval.empty() )
      {
        new Tag( l, "TYPE", m_logo.type );
        new Tag( l, "BINVAL", Base64::encode64( m_logo.binval ) );
      }
    }

    EmailList::const_iterator ite = m_emailList.begin();
    for( ; ite != m_emailList.end(); ++ite )
    {
      Tag* e = new Tag( v, "EMAIL" );
      insertField( e, "INTERNET", (*ite).internet );
      insertField( e, "WORK", (*ite).work );
      insertField( e, "HOME", (*ite).home );
      insertField( e, "X400", (*ite).x400 );
      insertField( e, "PREF", (*ite).pref );
      insertField( e, "USERID", (*ite).userid );
    }

    AddressList::const_iterator ita = m_addressList.begin();
    for( ; ita != m_addressList.end(); ++ita )
    {
      Tag* a = new Tag( v, "ADR" );
      insertField( a, "POSTAL", (*ita).postal );
      insertField( a, "PARCEL", (*ita).parcel );
      insertField( a, "HOME", (*ita).home );
      insertField( a, "WORK", (*ita).work );
      insertField( a, "PREF", (*ita).pref );
      insertField( a, "DOM", (*ita).dom );
      if( !(*ita).dom )
        insertField( a, "INTL", (*ita).intl );

      insertField( a, "POBOX", (*ita).pobox );
      insertField( a, "EXTADD", (*ita).extadd );
      insertField( a, "STREET", (*ita).street );
      insertField( a, "LOCALITY", (*ita).locality );
      insertField( a, "REGION", (*ita).region );
      insertField( a, "PCODE", (*ita).pcode );
      insertField( a, "CTRY", (*ita).ctry );
    }

    TelephoneList::const_iterator itt = m_telephoneList.begin();
    for( ; itt != m_telephoneList.end(); ++itt )
    {
      Tag* t = new Tag( v, "TEL" );
      insertField( t, "NUMBER", (*itt).number );
      insertField( t, "HOME", (*itt).home );
      insertField( t, "WORK", (*itt).work );
      insertField( t, "VOICE", (*itt).voice );
      insertField( t, "FAX", (*itt).fax );
      insertField( t, "PAGER", (*itt).pager );
      insertField( t, "MSG", (*itt).msg );
      insertField( t, "CELL", (*itt).cell );
      insertField( t, "VIDEO", (*itt).video );
      insertField( t, "BBS", (*itt).bbs );
      insertField( t, "MODEM", (*itt).modem );
      insertField( t, "ISDN", (*itt).isdn );
      insertField( t, "PCS", (*itt).pcs );
      insertField( t, "PREF", (*itt).pref );
    }

    if( !m_geo.latitude.empty() && !m_geo.longitude.empty() )
    {
      Tag* g = new Tag( v, "GEO" );
      new Tag( g, "LAT", m_geo.latitude );
      new Tag( g, "LON", m_geo.longitude );
    }

    if( !m_org.name.empty() )
    {
      Tag* o = new Tag( v, "ORG" );
      new Tag( o, "ORGNAME", m_org.name );
      StringList::const_iterator ito = m_org.units.begin();
      for( ; ito != m_org.units.end(); ++ito )
        new Tag( o, "ORGUNIT", (*ito) );
    }

    if( m_class != ClassNone )
    {
      Tag* c = new Tag( v, "CLASS" );
      switch( m_class )
      {
        case ClassPublic:
          new Tag( c, "PUBLIC" );
          break;
        case ClassPrivate:
          new Tag( c, "PRIVATE" );
          break;
        case ClassConfidential:
          new Tag( c, "CONFIDENTIAL" );
          break;
        default:
          break;
      }
    }

    return v;
  }
}
