/*
  Copyright (c) 2006-2009 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef VCARD_H__
#define VCARD_H__

#include "gloox.h"
#include "stanzaextension.h"

namespace gloox
{

  class Tag;

  /**
   * @brief A VCard abstraction.
   *
   * See @link gloox::VCardManager VCardManager @endlink for info on how to
   * fetch VCards.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API VCard : public StanzaExtension
  {
    public:
      /**
       * Addressing type indicators.
       * @note @c AddrTypeDom and @c AddrTypeIntl are mutually exclusive. If both are present,
       * @c AddrTypeDom takes precendence.
       * @note Also note that not all adress types are applicable everywhere. For example,
       * @c AddrTypeIsdn does not make sense for a postal address. Check XEP-0054
       * for details.
       */
      enum AddressType
      {
        AddrTypeHome   =      1,    /**< Home address. */
        AddrTypeWork   =      2,    /**< Work address. */
        AddrTypePref   =      4,    /**< Preferred address. */
        AddrTypeX400   =      8,    /**< X.400 address. */
        AddrTypeInet   =     16,    /**< Internet address. */
        AddrTypeParcel =     32,    /**< Parcel address. */
        AddrTypePostal =     64,    /**< Postal address. */
        AddrTypeDom    =    128,    /**< Domestic(?) address. */
        AddrTypeIntl   =    256,    /**< International(?) address. */
        AddrTypeVoice  =    512,    /**< Voice number. */
        AddrTypeFax    =   1024,    /**< Fax number. */
        AddrTypePager  =   2048,    /**< Pager. */
        AddrTypeMsg    =   4096,    /**< MSG(?) */
        AddrTypeCell   =   8192,    /**< Cell phone number. */
        AddrTypeVideo  =  16384,    /**< Video chat(?). */
        AddrTypeBbs    =  32768,    /**< BBS. */
        AddrTypeModem  =  65536,    /**< Modem. */
        AddrTypeIsdn   = 131072,    /**< ISDN. */
        AddrTypePcs    = 262144     /**< PCS. */
      };

      /**
       * A person's full name.
       */
      struct Name
      {
        std::string family;         /**< Family name. */
        std::string given;          /**< Given name. */
        std::string middle;         /**< Middle name. */
        std::string prefix;         /**< Name prefix. */
        std::string suffix;         /**< Name suffix. */
      };

      /**
       * Classifies the VCard.
       */
      enum VCardClassification
      {
        ClassNone         = 0,      /**< Not classified. */
        ClassPublic       = 1,      /**< Public. */
        ClassPrivate      = 2,      /**< Private. */
        ClassConfidential = 4       /**< Confidential. */
      };

      /**
       * Describes an email field.
       */
      struct Email
      {
        std::string userid;         /**< Email address. */
        bool home;                  /**< Whether this is a personal address. */
        bool work;                  /**< Whether this is a work address. */
        bool internet;              /**< Whether this is an internet address(?). */
        bool pref;                  /**< Whether this is the preferred address. */
        bool x400;                  /**< Whether this is an X.400 address. */
      };

      /**
       * A list of email fields.
       */
      typedef std::list<Email> EmailList;

      /**
       * Describes a telephone number entry.
       */
      struct Telephone
      {
        std::string number;         /**< The phone number. */
        bool home;                  /**< Whether this is a personal number. */
        bool work;                  /**< Whether this is a work number. */
        bool voice;                 /**< Whether this is a voice number. */
        bool fax;                   /**< Whether this is a fax number. */
        bool pager;                 /**< Whether this is a pager. */
        bool msg;                   /**< MSG(?) */
        bool cell;                  /**< Whether this is a cell phone. */
        bool video;                 /**< Whether this is a video chat(?). */
        bool bbs;                   /**< Whether this is a BBS. */
        bool modem;                 /**< Whether this is a modem. */
        bool isdn;                  /**< Whether this is a ISDN line(?) */
        bool pcs;                   /**< PCS(?) */
        bool pref;                  /**< Whether this is the preferred number. */
      };

      /**
       * A list of telephone entries.
       */
      typedef std::list<Telephone> TelephoneList;

      /**
       * Describes an address entry.
       */
      struct Address
      {
        std::string pobox;          /**< Pobox. */
        std::string extadd;         /**< Extended address. */
        std::string street;         /**< Street. */
        std::string locality;       /**< Locality. */
        std::string region;         /**< Region. */
        std::string pcode;          /**< Postal code. */
        std::string ctry;           /**< Country. */
        bool home;                  /**< Whether this is a personal address. */
        bool work;                  /**< Whether this is a work address. */
        bool postal;                /**< Whether this is a postal address(?). */
        bool parcel;                /**< Whether this is a arcel address(?). */
        bool pref;                  /**< Whether this is the preferred address. */
        bool dom;                   /**< Whether this is a domestic(?) address. */
        bool intl;                  /**< Whether this is an international(?) address. */
      };

      /**
       * Describes an address label.
       */
      struct Label
      {
        StringList lines;           /**< A list of lines. */
        bool home;                  /**< Whether this is a personal address. */
        bool work;                  /**< Whether this is a work address. */
        bool postal;                /**< Whether this is a postal address(?). */
        bool parcel;                /**< Whether this is a arcel address(?). */
        bool pref;                  /**< Whether this is the preferred address. */
        bool dom;                   /**< Whether this is a domestic(?) address. */
        bool intl;                  /**< Whether this is an international(?) address. */
      };

      /**
       * Describes geo information.
       */
      struct Geo
      {
        std::string latitude;       /**< Longitude. */
        std::string longitude;      /**< Latitude. */
      };

      /**
       * Describes organization information.
       */
      struct Org
      {
        std::string name;           /**< The organizations name. */
        StringList units;           /**< A list of units in the organization
                                     * (the VCard's owner belongs to?). */
      };

      /**
       * Describes photo/logo information.
       */
      struct Photo
      {
        std::string extval;         /**< The photo is not stored inside the VCard. This is a hint (URL?)
                                     * where to look for it. */
        std::string binval;         /**< This is the photo (binary). */
        std::string type;           /**< This is a hint at the mime-type. May be forged! */
      };

      /**
       * A list of address entries.
       */
      typedef std::list<Address> AddressList;

      /**
       * A list of address labels.
       */
      typedef std::list<Label> LabelList;

      /**
       * Constructor.
       */
      VCard();

      /**
       * Constructs a new VCard from a given Tag containing appropriate fields.
       * @param vcard The VCard-Tag.
       */
      VCard( const Tag* vcard );

      /**
       * Virtual destructor.
       */
      virtual ~VCard() {}

      /**
       * Sets the formatted name.
       * @param name The formatted name.
       */
      void setFormattedname( const std::string& name ) { m_formattedname = name; }

      /**
       * Returns the formatted name.
       * @return The formatted name.
       */
      const std::string& formattedname() const { return m_formattedname; }

      /**
       * Sets the individual name parts. Unused parts can be left empty.
       * @param family The family name.
       * @param given The given name.
       * @param middle The middle name(s)
       * @param prefix A name prefix.
       * @param suffix A name suffix.
       */
      void setName( const std::string& family, const std::string& given,
                    const std::string& middle = EmptyString,
                    const std::string& prefix = EmptyString,
                    const std::string& suffix = EmptyString );

      /**
       * Returns a full name.
       * @return A full name.
       */
      const Name& name() const { return m_name; }

      /**
       * Sets a nickname.
       * @param nickname The nickname.
       */
      void setNickname( const std::string& nickname ) { m_nickname = nickname; }

      /**
       * Returns the nickname.
       * @return The nickname.
       */
      const std::string& nickname() const { return m_nickname; }

      /**
       * Sets a URL (homepage, etc.).
       * @param url The URL.
       */
      void setUrl( const std::string& url ) { m_url = url; }

      /**
       * Returns the url.
       * @return The url.
       */
      const std::string& url() const { return m_url; }

      /**
       * Sets the birthday.
       * @param bday The birthday, ISO 8601 formatted.
       */
      void setBday( const std::string& bday ) { m_bday = bday; }

      /**
       * Returns the birthday.
       * @return The birthday.
       */
      const std::string& bday() const { return m_bday; }

      /**
       * Sets a Jabber ID.
       * @param jabberid The (bare) Jabber ID (node\@host).
       */
      void setJabberid( const std::string& jabberid ) { m_jabberid = jabberid; }

      /**
       * Returns the Jabber ID.
       * @return The Jabber ID.
       */
      const std::string& jabberid() const { return m_jabberid; }

      /**
       * Sets the person's title.
       * @param title The title.
       */
      void setTitle( const std::string& title ) { m_title = title; }

      /**
       * Returns the title.
       * @return The title.
       */
      const std::string& title() const { return m_title; }

      /**
       * Sets the person's role.
       * @param role The role.
       */
      void setRole( const std::string& role ) { m_role = role; }

      /**
       * Returns the role.
       * @return The role.
       */
      const std::string& role() const { return m_role; }

      /**
       * Sets a "Commentary note".
       * @param note The note.
       */
      void setNote( const std::string& note ) { m_note = note; }

      /**
       * Returns the note.
       * @return The note.
       */
      const std::string& note() const { return m_note; }

      /**
       * Sets a "free-form descriptive text".
       * @param desc The descriptive text.
       */
      void setDesc( const std::string& desc ) { m_desc = desc; }

      /**
       * Returns the "free-form descriptive text".
       * @return The descriptive text.
       */
      const std::string& desc() const { return m_desc; }

      /**
       * Sets the "Mailer (e.g., Mail User Agent Type) property".
       * @param mailer The mailer.
       */
      void setMailer( const std::string& mailer ) { m_mailer = mailer; }

      /**
       * Returns the mailer.
       * @return The mailer.
       */
      const std::string& mailer() const { return m_mailer; }

      /**
       * Sets the "Last revised property. The value must be an ISO 8601 formatted UTC date/time."
       * @param rev The date of the last revision.
       */
      void setRev( const std::string& rev ) { m_rev = rev; }

      /**
       * Returns the date of the last revision.
       * @return The date of the last revision.
       */
      const std::string& rev() const { return m_rev; }

      /**
       * Sets the "Unique identifier property."
       * @param uid The unique identifier.
       */
      void setUid( const std::string& uid ) { m_uid = uid; }

      /**
       * Returns the unique identifier.
       * @return The unique identifier.
       */
      const std::string& uid() const { return m_uid; }

      /**
       * Sets the Time zone's Standard Time UTC offset. Value must be an ISO 8601
       * formatted UTC offset.
       * @param tz The timezone offset.
       */
      void setTz( const std::string& tz ) { m_tz = tz; }

      /**
       * Returns the timezone offset.
       * @return The timezone offset.
       */
      const std::string& tz() const { return m_tz; }

      /**
       * Sets the "Identifier of product that generated the vCard property."
       * @param prodid The product identifier.
       */
      void setProdid( const std::string& prodid ) { m_prodid = prodid; }

      /**
       * Returns the product identifier.
       * @return The product identifier.
       */
      const std::string& prodid() const { return m_prodid; }

      /**
       * Sets the "Sort string property."
       * @param sortstring The sort string.
       */
      void setSortstring( const std::string& sortstring ) { m_sortstring = sortstring; }

      /**
       * Returns the sort string.
       * @return The sort string.
       */
      const std::string& sortstring() const { return m_sortstring; }

      /**
       * Sets a URI to a photo.
       * @param extval The URI to the photo.
       */
      void setPhoto( const std::string& extval );

      /**
       * Sets the photo directly.
       * @param type Format type parameter.
       * @param binval The binary photo data.
       * @note To remove the photo from the VCard call this function without any parameters.
       */
      void setPhoto( const std::string& type = EmptyString, const std::string& binval = EmptyString );

      /**
       * Returns photo information.
       * @return Info about the photo.
       */
      const Photo& photo() const { return m_photo; }

      /**
       * Sets a URI to a organization logo.
       * @param extval The URI to the logo.
       */
      void setLogo( const std::string& extval );

      /**
       * Sets the organization logo directly.
       * @param type Format type parameter.
       * @param binval The binary logo data.
       * @note To remove the logo from the VCard call this function without any parameters.
       */
      void setLogo( const std::string& type = EmptyString, const std::string& binval = EmptyString );

      /**
       * Returns logo information.
       * @return Info about the logo.
       */
      const Photo& logo() const { return m_logo; }

      /**
       * Adds an email address.
       * @param userid The email address.
       * @param type Bit-wise ORed @ref AddressType.
       */
      void addEmail( const std::string& userid, int type );

      /**
       * Returns a list of email addresses.
       * @return A list of email addresses.
       */
      EmailList& emailAddresses() { return m_emailList; }

      /**
       * Adds an address.
       * @param pobox A PO Box.
       * @param extadd Some extra field (extended address).
       * @param street The street.
       * @param locality The village, city, etc.
       * @param region The region.
       * @param pcode The post code.
       * @param ctry The country.
       * @param type Bit-wise ORed @ref AddressType.
       */
      void addAddress( const std::string& pobox, const std::string& extadd,
                       const std::string& street, const std::string& locality,
                       const std::string& region, const std::string& pcode,
                       const std::string& ctry, int type );

      /**
       * Adds an address label.
       * @param lines A list of address label lines.
       * @param type Bit-wise ORed @c AddressType.
       */
      void addLabel( const StringList& lines, int type );

      /**
       * Returns a list of addresses.
       * @return A list of addresses.
       */
      AddressList& addresses() { return m_addressList; }

      /**
       * Returns a list of address labels.
       * @return A list of address labels.
       */
      LabelList& labels() { return m_labelList; }

      /**
       * Adds a telephone number.
       * @param number The telephone number.
       * @param type Bit-wise ORed @ref AddressType.
       */
      void addTelephone( const std::string& number, int type );

      /**
       * Returns a list of telephone numbers.
       * @return A list of telephone numbers.
       */
      TelephoneList& telephone() { return m_telephoneList; }

      /**
       * Sets "Geographical position. Values are the decimal degrees of
       * Latitude and Longitude. The value should be specified to six decimal places."
       * @param lat The latitude.
       * @param lon The longitude.
       */
      void setGeo( const std::string& lat, const std::string& lon );

      /**
       * Returns geo location information.
       * @return Info about the geo location.
       */
      const Geo& geo() const { return m_geo; }

      /**
       * Sets "Organizational name and units property."
       * @param orgname The organization's name.
       * @param orgunits A list of organizational units.
       */
      void setOrganization( const std::string& orgname, const StringList& orgunits );

      /**
       * Returns organization information.
       * @return Info about the organization.
       */
      const Org& org() const { return m_org; }

      /**
       * Sets the "Privacy classification property."
       * @param vclass The classification value.
       */
      void setClass( VCardClassification vclass ) { m_class = vclass; }

      /**
       * Returns information about classification.
       * @return Info about the classification.
       */
      VCardClassification classification() const { return m_class; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new VCard( tag );
      }

      // reimplemented from StanzaExtension
      virtual Tag* tag() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new VCard( *this );
      }

    private:

      static void insertField( Tag* vcard, const char* field, const std::string& var );
      static void insertField( Tag* vcard, const char* field, bool var );
      static void checkField ( const Tag* vcard, const char* field, std::string& var );

      EmailList m_emailList;
      TelephoneList m_telephoneList;
      AddressList m_addressList;
      LabelList m_labelList;

      Name m_name;
      Geo m_geo;
      Org m_org;
      Photo m_photo;
      Photo m_logo;

      VCardClassification m_class;

      std::string m_formattedname;
      std::string m_nickname;
      std::string m_url;
      std::string m_bday;
      std::string m_jabberid;
      std::string m_title;
      std::string m_role;
      std::string m_note;
      std::string m_desc;
      std::string m_mailer;
      std::string m_tz;
      std::string m_prodid;
      std::string m_rev;
      std::string m_sortstring;
      std::string m_uid;

      bool m_N;
      bool m_PHOTO;
      bool m_LOGO;
  };

}

#endif // VCARD_H__
